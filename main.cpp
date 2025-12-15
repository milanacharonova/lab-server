#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include <algorithm>

using namespace std;
using boost::asio::ip::tcp;

class car {
    std::mutex db_mutex;
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
    void display();
    void find();
    void add();
    void del();
    void redact();
};

istream& operator>>(istream& in, car& p) {
    in >> p._brand >> p._model >> p._km >> p._cu;
    return in;
}

ostream& operator<<(ostream& out, car& p) {
    out << p._brand << " " << p._model << " " << p._km << " " << p._cu;
    return out;
}

void car::display() {
    std::lock_guard<std::mutex> lock(db_mutex);
    string line;
    ifstream file("data.txt");
    if (file.is_open()) {
        cout << "-------------------------------" << endl;
        while (getline(file, line)) {
            cout << line << endl;
        }
        file.close();
        cout << "-------------------------------" << endl;
        return;
    }
    else {
        cout << "Cant open file" << endl;
        return;
    }
}

void car::find() {
    std::lock_guard<std::mutex> lock(db_mutex);
    int flag = 0;
    ifstream file("data.txt");
    if (file.is_open()) {
        int ch;
        cout << "Choose what to search\n1 - Brand\n2 - Model\n3 - KM\n4 - Current user\n" << endl;
        cin >> ch;
        switch (ch) {
        case 1: {
            cout << "Type brand:";
            string brand;
            cin >> brand;
            cout << "-------------------------------" << endl;
            string line;
            while (file >> _brand >> _model >> _km >> _cu) {
                if (brand == _brand) {
                    cout << _brand << " " << _model << " " << _km << " " << _cu << endl;
                    flag = 1;
                }
            }
            if (flag == 0) {
                cout << "Nothing found :(" << endl;
            }
            cout << "-------------------------------" << endl;
            file.close();
            break;
        }
        case 2: {
            cout << "Type model:";
            string model;
            cin >> model;
            cout << "-------------------------------" << endl;
            string line;
            while (file >> _brand >> _model >> _km >> _cu) {
                if (model == _model) {
                    cout << _brand << " " << _model << " " << _km << " " << _cu << endl;
                    flag = 1;
                }
            }
            if (flag == 0) {
                cout << "Nothing found :(" << endl;
            }
            cout << "-------------------------------" << endl;
            file.close();
            break;
        }
        case 3: {
            cout << "Type KM:";
            int KM;
            cin >> KM;
            cout << "-------------------------------" << endl;
            string line;
            while (file >> _brand >> _model >> _km >> _cu) {
                if (KM == _km) {
                    cout << _brand << " " << _model << " " << _km << " " << _cu << endl;
                    flag = 1;
                }
            }
            if (flag == 0) {
                cout << "Nothing found :(" << endl;
            }
            cout << "-------------------------------" << endl;
            file.close();
            break;
        }
        case 4: {
            cout << "Type Current User:";
            string cu;
            cin >> cu;
            cout << "-------------------------------" << endl;
            string line;
            while (file >> _brand >> _model >> _km >> _cu) {
                if (cu == _cu) {
                    cout << _brand << " " << _model << " " << _km << " " << _cu << endl;
                    flag = 1;
                }
            }
            if (flag == 0) {
                cout << "Nothing found :(" << endl;
            }
            cout << "-------------------------------" << endl;
            file.close();
            break;
        }
        default: {
            break;
        }
        }
    }
    else {
        cout << "Cant open file";
    }
}

void car::add() {
    std::lock_guard<std::mutex> lock(db_mutex);
    cout << "Enter brand of car: ";
    cin >> _brand;
    cout << "Enter model of car: ";
    cin >> _model;
    cout << "Enter how many km did car ride: ";
    cin >> _km;
    cout << "Enter current user of car: ";
    cin >> _cu;
    ofstream file("data.txt", ios::app);
    if (file.is_open()) {
        file << _brand << " " << _model << " " << _km << " " << _cu << endl;
    }
    file.close();
}

void car::del() {
    std::lock_guard<std::mutex> lock(db_mutex);
    int count = 0;
    ifstream file("data.txt");
    if (file.is_open()) {
        while (file >> _brand >> _model >> _km >> _cu) {
            count++;
        }
    }
    else {
        cout << "Cant open file :(" << endl;
        return;
    }
    car* arr = new car[count];
    cout << "-------------------------------" << endl;
    ifstream f("data.txt");
    if (f.is_open()) {
        for (int i = 0; i < count; i++) {
            f >> arr[i]._brand >> arr[i]._model >> arr[i]._km >> arr[i]._cu;
            cout << i << ")" << " " << arr[i]._brand << " " << arr[i]._model << " " << arr[i]._km << " " << arr[i]._cu << endl;
        }
    }
    else {
        cout << "Cant open file :(" << endl;
        delete[] arr;
        return;
    }
    f.close();
    file.close();
    cout << "-------------------------------" << endl;
    cout << "Enter number of line u want to del: ";
    int num;
    cin >> num;
    ofstream file1("data.txt");
    if (file1.is_open()) {
        for (int i = 0; i < count; i++) {
            if (i == num) {
            }
            else {
                file1 << arr[i]._brand << " " << arr[i]._model << " " << arr[i]._km << " " << arr[i]._cu << endl;
            }
        }
    }
    else {
        cout << "Cant open file :(" << endl;
    }
    file1.close();
    delete[] arr;
}

void car::redact() {
    std::lock_guard<std::mutex> lock(db_mutex);
    int count = 0;
    ifstream file("data.txt");
    if (file.is_open()) {
        while (file >> _brand >> _model >> _km >> _cu) {
            count++;
        }
    }
    else {
        cout << "Cant open file :(" << endl;
        return;
    }
    car* arr = new car[count];
    cout << "-------------------------------" << endl;
    ifstream f("data.txt");
    if (f.is_open()) {
        for (int i = 0; i < count; i++) {
            f >> arr[i]._brand >> arr[i]._model >> arr[i]._km >> arr[i]._cu;
            cout << i << ")" << " " << arr[i]._brand << " " << arr[i]._model << " " << arr[i]._km << " " << arr[i]._cu << endl;
        }
    }
    else {
        cout << "Cant open file :(" << endl;
        delete[] arr;
        return;
    }
    f.close();
    file.close();
    cout << "-------------------------------" << endl;
    cout << "Enter number of line u want to edit: ";
    int num;
    cin >> num;
    cout << "Enter brand of car: ";
    cin >> _brand;
    cout << "Enter model of car: ";
    cin >> _model;
    cout << "Enter how many km did car ride: ";
    cin >> _km;
    cout << "Enter current user of car: ";
    cin >> _cu;
    ofstream file1("data.txt");
    if (file1.is_open()) {
        for (int i = 0; i < count; i++) {
            if (i == num) {
                file1 << _brand << " " << _model << " " << _km << " " << _cu << endl;
            }
            else {
                file1 << arr[i]._brand << " " << arr[i]._model << " " << arr[i]._km << " " << arr[i]._cu << endl;
            }
        }
    }
    else {
        cout << "Cant open file :(" << endl;
    }
    file1.close();
    delete[] arr;
}

class server {
private:
    boost::asio::io_context io_context;
    tcp::acceptor acceptor;
    vector<shared_ptr<tcp::socket>> clients;
    mutex clients_mutex;
public:
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
                string menu = "----------menu-----------\n";
                boost::asio::async_write(*socket, boost::asio::buffer(menu),
                    [](const boost::system::error_code&, size_t) {});
                user_input(socket);
            }
            accept_connections();
        });
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
                        taxi.display();
                    }
                    else if (input == "2") {
                        taxi.find();
                    }
                    else if (input == "3") {
                        taxi.add();
                    }
                    else if (input == "4") {
                        taxi.del();
                    }
                    else if (input == "5") {
                        taxi.redact();
                    }
                    else if (input == "6") {
                        boost::system::error_code close_ec;
                        socket->close(close_ec);
                        return;
                    }
                    else {
                        string error = "Unknown command\n";
                        boost::asio::async_write(*socket, boost::asio::buffer(error),
                            [](const boost::system::error_code&, size_t) {});
                    }
                }
                this->user_input(socket);
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
};

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
