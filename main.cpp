#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include <algorithm>
#include <functional>
#include <sstream>

using namespace std;
using boost::asio::ip::tcp;

class server;
class car {
    string _brand;
    string _model;
    int _km;
    string _cu;
public:
    car() = default;
    car(string brand, string model, int km, string cu)
        : _brand(brand), _model(model), _km(km), _cu(cu) {}
    friend istream& operator>>(istream& in, car& p);
    friend ostream& operator<<(ostream& out, car& p);
    void display(shared_ptr<tcp::socket> socket, server* srv);
    void add(shared_ptr<tcp::socket> socket, server* srv);
    void del(shared_ptr<tcp::socket> socket, server* srv);
};

istream& operator>>(istream& in, car& p) {
    in >> p._brand >> p._model >> p._km >> p._cu;
    return in;
}

ostream& operator<<(ostream& out, car& p) {
    out << p._brand << " " << p._model << " " << p._km << " " << p._cu;
    return out;
}

class server {
private:
    boost::asio::io_context io_context;
    tcp::acceptor acceptor;
    vector<shared_ptr<tcp::socket>> clients;
    // Добавлен мьютекс для защиты операций с базой данных
public:
    mutex clients_mutex;
    mutex db_mutex;
    server(uint16_t port) : acceptor(io_context, tcp::endpoint(tcp::v4(), port)) {
        cout << "Сервер запущен на порту:" << port << endl;
        cout << "-------------------------------" << endl;
        accept_connections();
    }

    void accept_connections() {
        auto socket = make_shared<tcp::socket>(io_context);
        acceptor.async_accept(*socket, [this, socket](const boost::system::error_code& ec) {
            if (!ec) {
                cout << "Новое подключение:" << socket->remote_endpoint() << endl;
                {
                    lock_guard<mutex> lock(clients_mutex);
                    clients.push_back(socket);
                }
                show_menu(socket);
                user_input(socket);
            }
            accept_connections();
        });
    }

    void show_menu(shared_ptr<tcp::socket> socket) {
        string menu = "----------menu-----------\n"
                      "1 - Display all cars\n"
                      "2 - Add car\n"
                      "3 - Delete car\n"
                      "4 - Edit car\n"
                      "6 - Exit\n"
                      "-------------------------\n"
                      "Enter your choice: ";
        boost::asio::async_write(*socket, boost::asio::buffer(menu),
            [](const boost::system::error_code&, size_t) {});
    }

    void user_input(shared_ptr<tcp::socket> socket) {
        auto buffer = make_shared<string>(1024, '\0');
        socket->async_read_some(boost::asio::buffer(*buffer), [this, socket, buffer](const boost::system::error_code& ec, size_t bytes_transferred) {
            if (!ec && bytes_transferred > 0) {
                buffer->resize(bytes_transferred);
                while (!buffer->empty() && (buffer->back() == '\n' || buffer->back() == '\r')) {
                    buffer->pop_back();
                }
                string input = *buffer;
                if (!input.empty()) {
                    cout << "[" << socket->remote_endpoint() << "] " << input << endl;
                    car taxi;
                    if (input == "1") {
                        taxi.display(socket, this);
                    }
                    else if (input == "2") {
                        taxi.add(socket, this);
                    }
                    else if (input == "3") {
                        taxi.del(socket, this);
                    }
                    else if (input == "6") {
                        string goodbye = "Goodbye!\n";
                        boost::asio::async_write(*socket, boost::asio::buffer(goodbye),
                            [socket](const boost::system::error_code&, size_t) {
                                boost::system::error_code close_ec;
                                socket->close(close_ec);
                            });
                        return;
                    }
                    else {
                        string error = "Unknown command. Try again:\n";
                        boost::asio::async_write(*socket, boost::asio::buffer(error),
                            [this, socket](const boost::system::error_code&, size_t) {
                                this->show_menu(socket);
                            });
                    }
                }
            }
            else {
                cout << "Клиент отключился:" << socket->remote_endpoint() << endl;
                {
                    lock_guard<mutex> lock(this->clients_mutex);
                    this->clients.erase(remove(this->clients.begin(), this->clients.end(), socket), this->clients.end());
                }
                boost::system::error_code close_ec;
                socket->close(close_ec);
            }
        });
    }

    void run() {
        this->io_context.run();
    }

    void ip_input(shared_ptr<tcp::socket> socket, function<void(string)> callback) {
        auto buffer = make_shared<string>(1024, '\0');
        socket->async_read_some(boost::asio::buffer(*buffer),
            [this, socket, buffer, callback](const boost::system::error_code& ec, size_t bytes_transferred) {
                if (!ec && bytes_transferred > 0) {
                    buffer->resize(bytes_transferred);
                    while (!buffer->empty() && (buffer->back() == '\n' || buffer->back() == '\r')) {
                        buffer->pop_back();
                    }
                    if (!buffer->empty()) {
                        callback(*buffer);
                    } else {
                        this->ip_input(socket, callback);
                    }
                } else {
                    boost::system::error_code close_ec;
                    socket->close(close_ec);
                }
            });
    }
};

void car::display(shared_ptr<tcp::socket> socket, server* srv) {
    // Используем мьютекс сервера для защиты доступа к файлу
    std::lock_guard<std::mutex> lock(srv->db_mutex);
    string response = "-------------------------------\n";
    ifstream file("data.txt");

    if (file.is_open()) {
        string line;
        bool found = false;
        while (getline(file, line)) {
            response += line + "\n";
            found = true;
        }
        file.close();

        if (!found) {
            response += "Database is empty\n";
        }
    } else {
        response = "Error: Can't open file\n";
    }

    response += "-------------------------------\n";

    boost::asio::async_write(*socket, boost::asio::buffer(response),
        [srv, socket](const boost::system::error_code&, size_t) {
            srv->show_menu(socket);
        });
}

void car::add(shared_ptr<tcp::socket> socket, server* srv) {
    string prompt = "Enter brand model km current_user (separated by spaces):\n";
    boost::asio::async_write(*socket, boost::asio::buffer(prompt),
        [this, socket, srv](const boost::system::error_code& ec, size_t) {
            if (!ec) {
                srv->ip_input(socket, [this, socket, srv](const string& input) {
                    istringstream iss(input);
                    string brand, model, cu;
                    int km;

                    if (iss >> brand >> model >> km >> cu) {
                        {
                            // Используем мьютекс сервера для защиты записи в файл
                            std::lock_guard<std::mutex> lock(srv->db_mutex);
                            ofstream file("data.txt", ios::app);
                            if (file.is_open()) {
                                file << brand << " " << model << " " << km << " " << cu << endl;
                                file.close();
                            }
                        }
                        string response = "Car added successfully!\n";
                        boost::asio::async_write(*socket, boost::asio::buffer(response),
                            [srv, socket](const boost::system::error_code&, size_t) {
                                srv->show_menu(socket);
                            });
                    } else {
                        string error = "Invalid format. Use: brand model km user\n";
                        boost::asio::async_write(*socket, boost::asio::buffer(error),
                            [this, socket, srv](const boost::system::error_code&, size_t) {
                                this->add(socket, srv);
                            });
                    }
                });
            }
        });
}

void car::del(shared_ptr<tcp::socket> socket, server* srv) {
    // Используем мьютекс сервера для защиты чтения файла
    std::lock_guard<std::mutex> lock(srv->db_mutex);
    ifstream file("data.txt");
    vector<car> cars;

    if (file.is_open()) {
        string brand, model, cu;
        int km;
        while (file >> brand >> model >> km >> cu) {
            cars.push_back(car(brand, model, km, cu));
        }
        file.close();
    }

    if (cars.empty()) {
        string response = "Database is empty. Nothing to delete.\n";
        boost::asio::async_write(*socket, boost::asio::buffer(response),
            [srv, socket](const boost::system::error_code&, size_t) {
                srv->show_menu(socket);
            });
        return;
    }

    // Показываем список машин
    string list = "-------------------------------\n";
    for (size_t i = 0; i < cars.size(); i++) {
        list += to_string(i) + ") " + cars[i]._brand + " " + cars[i]._model +
                " " + to_string(cars[i]._km) + " " + cars[i]._cu + "\n";
    }
    list += "-------------------------------\n";
    list += "Enter number of car to delete: ";

    boost::asio::async_write(*socket, boost::asio::buffer(list),
        [this, socket, srv, cars](const boost::system::error_code& ec, size_t) {
            if (!ec) {
                srv->ip_input(socket, [this, socket, srv, cars](const string& input) {
                    try {
                        size_t num = stoul(input);
                        if (num >= cars.size()) {
                            string error = "Invalid number. Try again:\n";
                            boost::asio::async_write(*socket, boost::asio::buffer(error),
                                [this, socket, srv, cars](const boost::system::error_code&, size_t) {
                                    this->del(socket, srv); // Перезапускаем удаление
                                });
                            return;
                        }

                        {
                            // Используем мьютекс сервера для защиты записи в файл
                            std::lock_guard<std::mutex> lock(srv->db_mutex);
                            ofstream file("data.txt");
                            for (size_t i = 0; i < cars.size(); i++) {
                                if (i != num) {
                                    file << cars[i]._brand << " " << cars[i]._model << " "
                                         << cars[i]._km << " " << cars[i]._cu << endl;
                                }
                            }
                        }

                        string response = "Car deleted successfully!\n";
                        boost::asio::async_write(*socket, boost::asio::buffer(response),
                            [srv, socket](const boost::system::error_code&, size_t) {
                                srv->show_menu(socket);
                            });
                    } catch (...) {
                        string error = "Invalid input. Enter a number:\n";
                        boost::asio::async_write(*socket, boost::asio::buffer(error),
                            [this, socket, srv, cars](const boost::system::error_code&, size_t) {
                                this->del(socket, srv); // Перезапускаем удаление
                            });
                    }
                });
            }
        });
}

int main() {
    try {
        const uint16_t port = 15001;
        server server(port);
        thread server_thread([&server]() {
            server.run();
        });

        cout << "Сервер работает в фоновом режиме. Нажмите Enter для завершения..." << endl;
        cin.get();
        server_thread.join();

    } catch (const exception& e) {
        cerr << "Исключение: " << e.what() << endl;
        return 1;
    }
    return 0;
}
