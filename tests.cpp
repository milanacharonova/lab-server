#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>
#include <filesystem> // для удаления файла
#include <random>
#include <chrono>

// Копия структуры car из основного кода
struct car {
    std::string _brand;
    std::string _model;
    int _km;
    std::string _cu;

    car() = default;
    car(std::string brand, std::string model, int km, std::string cu)
        : _brand(std::move(brand)), _model(std::move(model)), _km(km), _cu(std::move(cu)) {}

    friend std::istream& operator>>(std::istream& in, car& p);
    friend std::ostream& operator<<(std::ostream& out, car& p);
};

std::istream& operator>>(std::istream& in, car& p) {
    in >> p._brand >> p._model >> p._km >> p._cu;
    return in;
}

std::ostream& operator<<(std::ostream& out, car& p) {
    out << p._brand << " " << p._model << " " << p._km << " " << p._cu;
    return out;
}

// Генерация уникального имени временного файла
std::string generate_temp_filename() {
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    auto rand = std::random_device{}();
    return "test_data_" + std::to_string(now) + "_" + std::to_string(rand) + ".txt";
}

// Тестовый "сервер" — только логика работы с файлом
struct test_server {
    std::mutex db_mutex;

    void add_car_to_file(const std::string& filename, const car& c) {
        std::lock_guard<std::mutex> lock(db_mutex);
        std::ofstream file(filename, std::ios::app);
        REQUIRE(file.is_open());
        file << c._brand << " " << c._model << " " << c._km << " " << c._cu << "\n";
        file.close();
    }

    std::vector<car> read_all_cars(const std::string& filename) {
        std::lock_guard<std::mutex> lock(db_mutex);
        std::vector<car> cars;
        std::ifstream file(filename);
        if (!file.is_open()) {
            return cars; // пустой вектор
        }
        car c;
        while (file >> c._brand >> c._model >> c._km >> c._cu) {
            cars.push_back(c);
        }
        file.close();
        return cars;
    }

    void delete_car_by_index(const std::string& filename, size_t index) {
        std::lock_guard<std::mutex> lock(db_mutex);
        auto cars = read_all_cars(filename);
        if (index >= cars.size()) return;

        std::ofstream file(filename);
        REQUIRE(file.is_open());
        for (size_t i = 0; i < cars.size(); ++i) {
            if (i != index) {
                file << cars[i]._brand << " " << cars[i]._model << " "
                     << cars[i]._km << " " << cars[i]._cu << "\n";
            }
        }
        file.close();
    }
};

// --- ТЕСТЫ ---

TEST_CASE("Car stream operators work correctly", "[car]") {
    car c("Toyota", "Camry", 50000, "user1");

    std::ostringstream oss;
    oss << c;
    REQUIRE(oss.str() == "Toyota Camry 50000 user1");

    std::istringstream iss("Honda Civic 30000 user2");
    car c2;
    iss >> c2;
    REQUIRE(c2._brand == "Honda");
    REQUIRE(c2._model == "Civic");
    REQUIRE(c2._km == 30000);
    REQUIRE(c2._cu == "user2");
}

TEST_CASE("Server can add a car to file", "[server]") {
    auto filename = generate_temp_filename();
    test_server srv;

    car c("BMW", "X5", 20000, "admin");
    srv.add_car_to_file(filename, c);

    auto cars = srv.read_all_cars(filename);
    REQUIRE(cars.size() == 1);
    REQUIRE(cars[0]._brand == "BMW");
    REQUIRE(cars[0]._model == "X5");
    REQUIRE(cars[0]._km == 20000);
    REQUIRE(cars[0]._cu == "admin");

    std::filesystem::remove(filename);
}

TEST_CASE("Server can read multiple cars from file", "[server]") {
    auto filename = generate_temp_filename();
    test_server srv;

    srv.add_car_to_file(filename, car("Audi", "A4", 15000, "user1"));
    srv.add_car_to_file(filename, car("Mercedes", "C-Class", 25000, "user2"));

    auto cars = srv.read_all_cars(filename);
    REQUIRE(cars.size() == 2);
    REQUIRE(cars[0]._brand == "Audi");
    REQUIRE(cars[1]._brand == "Mercedes");

    std::filesystem::remove(filename);
}

TEST_CASE("Server can delete car by index", "[server]") {
    auto filename = generate_temp_filename();
    test_server srv;

    srv.add_car_to_file(filename, car("Ford", "Focus", 10000, "user1"));
    srv.add_car_to_file(filename, car("Chevrolet", "Malibu", 12000, "user2"));
    srv.add_car_to_file(filename, car("Volkswagen", "Golf", 8000, "user3"));

    auto before = srv.read_all_cars(filename);
    REQUIRE(before.size() == 3);

    srv.delete_car_by_index(filename, 1); // Удаляем Chevrolet

    auto after = srv.read_all_cars(filename);
    REQUIRE(after.size() == 2);
    REQUIRE(after[0]._brand == "Ford");
    REQUIRE(after[1]._brand == "Volkswagen");

    std::filesystem::remove(filename);
}

TEST_CASE("Deleting invalid index does nothing", "[server]") {
    auto filename = generate_temp_filename();
    test_server srv;

    srv.add_car_to_file(filename, car("Tesla", "Model S", 5000, "user1"));

    // Попытка удалить несуществующий индекс
    srv.delete_car_by_index(filename, 999);

    auto after = srv.read_all_cars(filename);
    REQUIRE(after.size() == 1); // данные не изменились

    std::filesystem::remove(filename);
}
