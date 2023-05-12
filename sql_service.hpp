#pragma once 

#include <iostream>
#include <string>
#include <vector>

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

#include "item.hpp"
#include "client.hpp"

class sql_service {

public:

	sql_service() {
        
        std::cout << "Enter mysql password:\n";
        std::cin >> password;
    }

    void Connect(const std::string& ip, const size_t& port_num) {

		connection = driver->connect("tcp://" + ip + ":" + std::to_string(port_num), "root", password);
	    CreateDatabase();
        connection->setSchema("store");    
    }

	// Adds new item to 'catalog' database
    void AddToCatalog(const Item& item) {
        
        ThrowIfItemExists(item.GetName());

        sql::PreparedStatement* stmt = connection->prepareStatement("INSERT INTO catalog(id, name, price, items_left)"
                                                                    " VALUES(NULL, (?), (?), (?))");
        stmt->setString(1, item.GetName());
        stmt->setInt(2, item.GetPrice());
        stmt->setInt(3, item.GetAmmount());
        stmt->execute();
	}

    void AddToCatalog(const std::vector<Item>& Items) {
        for (auto& item : Items) {
            AddToCatalog(item);
        }
    }

	void IncreaseNumberOfItems(const std::string& item_name, const size_t& change_item_ammount) {
        
        ThrowIfNoItem(item_name);
        sql::PreparedStatement* update_ammount = connection->prepareStatement("UPDATE catalog SET items_left = items_left + (?) "
                                                                              " WHERE name = (?)"); 
        update_ammount->setString(2, item_name);
        update_ammount->setInt(1, change_item_ammount);
        update_ammount->execute();
    }

	void DecreaseNumberOfItems(const std::string& item_name, const size_t& change_item_ammount) {
        
        CheckDecreaseNumberOfItems(item_name, change_item_ammount);
        sql::PreparedStatement* update_ammount = connection->prepareStatement("UPDATE catalog SET items_left = items_left - (?) "
                                                                              " WHERE name = (?)"); 
        update_ammount->setString(2, item_name);
        update_ammount->setInt(1, change_item_ammount);
        update_ammount->execute();
    }


	void MakeOrder(const std::string& client_mail, std::vector<Item>& order) {
        
        for (auto& item : order) {
            DecreaseNumberOfItems(item.GetName(), item.GetAmmount());
        }
        
        size_t total_order = 0, total_items = 0, discount = GetClientDiscount(client_mail);
        for (auto& item : order) {
            total_order += GetItemPrice(item.GetName()) * item.GetAmmount();
            total_items += item.GetAmmount();
        }
        size_t total_with_discount = total_order - total_order * discount / 100; 
        AddOrder(client_mail, total_items, total_order, total_with_discount);
        UpdateClientOrders(client_mail, total_with_discount);
        UpdateClientDiscount(client_mail);
        
    }

    void AddClient(const Client& client) {
        
        ThrowIfClientExists(client.GetMail());
        sql::PreparedStatement* add_client = connection->prepareStatement("INSERT INTO clients VALUES (NULL, ?, ?, ?, ?, ?)");
        add_client->setString(1, client.GetName());
        add_client->setString(2, client.GetMail());
        add_client->setInt(3, 0);
        add_client->setInt(4, 0);
        add_client->setInt(5, 0);
        add_client->execute();
    }

    void AddClients(const std::vector<Client>& clients) {

    void AddClients(const std::vector<Client>& clients) {
        for (auto& client : clients) {
            AddClient(client);
        }
    }

	// Adds order to 'order' table
	void AddOrder(const std::string& client_mail, const size_t& total_items, const size_t& total_order, const size_t& total_with_discount) {
        
        sql::PreparedStatement* add_order = connection->prepareStatement("INSERT INTO orders VALUES (NULL, (?), NOW(), (?), (?), (?))");
        add_order->setString(1, client_mail);
        add_order->setInt(2, total_items);
        add_order->setInt(3, total_order);
        add_order->setInt(4, total_with_discount);
        add_order->execute();
    }


    size_t GetClientDiscount(const std::string& client_mail) {
        ThrowIfNoClient(client_mail);
        sql::PreparedStatement* get_discount = connection->prepareStatement("SELECT discount FROM clients WHERE mail = (?)");
        get_discount->setString(1, client_mail);
        sql::ResultSet* discount_res = get_discount->executeQuery();
        if (discount_res->next()) {
            return discount_res->getInt(1);
        }
    }

    size_t GetClientTotalSpent(const std::string& client_mail) {
        ThrowIfNoClient(client_mail);
        sql::PreparedStatement* get_discount = connection->prepareStatement("SELECT total_spent FROM clients WHERE mail = (?)");
        get_discount->setString(1, client_mail);
        sql::ResultSet* discount_res = get_discount->executeQuery();
        if (discount_res->next()) {
            return discount_res->getInt(1);
        }
    }

    size_t GetClientTotalOrders(const std::string& client_mail) {
        ThrowIfNoClient(client_mail);
        sql::PreparedStatement* get_total_orders = connection->prepareStatement("SELECT total_orders FROM clients WHERE mail = (?)");
        get_total_orders->setString(1, client_mail);
        sql::ResultSet* total_orders = get_total_orders->executeQuery();
        if (total_orders->next()) {
            return total_orders->getInt(1);
        }
    }


    std::string GetClientName(const std::string& client_mail) {
        ThrowIfNoClient(client_mail);
        sql::PreparedStatement* get_name = connection->prepareStatement("SELECT name FROM clients WHERE mail = (?)");
        get_name->setString(1, client_mail);
        sql::ResultSet* res = get_name->executeQuery();
        if (res->next()) 
            return res->getString(1);
    }

    std::string GetOrderMail(const size_t& order_id) {
        ThrowIfNoOrder(order_id);
        sql::PreparedStatement* get_mail = connection->prepareStatement("SELECT mail FROM orders WHERE id = (?)");
        get_mail->setInt(1, order_id);
        sql::ResultSet* res = get_mail->executeQuery();
        if (res->next()) {
            return res->getString(1);
        }
    }

    size_t GetOrderNumberOfItems(const size_t& order_id) {
        ThrowIfNoOrder(order_id);
        sql::PreparedStatement* get_number_of_items = connection->prepareStatement("SELECT number_of_items FROM orders WHERE id = (?)");
        get_number_of_items->setInt(1, order_id);
        sql::ResultSet* res = get_number_of_items->executeQuery();
        if (res->next()) {
            return res->getInt(1);
        }
    }

    size_t GetOrderTotalNoDiscount(const size_t& order_id) {
        ThrowIfNoOrder(order_id);
        sql::PreparedStatement* get_total = connection->prepareStatement("SELECT total_no_discount FROM orders WHERE id = (?)");
        get_total->setInt(1, order_id);
        sql::ResultSet* res = get_total->executeQuery();
        if (res->next()) {
            return res->getInt(1);
        }
    }

    size_t GetOrderTotalWithDiscount(const size_t& order_id) {
        ThrowIfNoOrder(order_id);
        sql::PreparedStatement* get_total = connection->prepareStatement("SELECT total_with_discount FROM orders WHERE id = (?)");
        get_total->setInt(1, order_id);
        sql::ResultSet* res = get_total->executeQuery();
        if (res->next()) {
            return res->getInt(1);
        }
    }

    void RemoveClient(const std::string& client_mail) {
        ThrowIfNoClient(client_mail);
        sql::PreparedStatement* remove_client = connection->prepareStatement("DELETE FROM clients WHERE mail = (?)");
        remove_client->setString(1, client_mail);
        remove_client->execute();
    }

    void RemoveItem(const std::string& item_name) {
        ThrowIfNoItem(item_name);
        sql::PreparedStatement* remove_item = connection->prepareStatement("DELETE FROM catalog WHERE name = (?)");
        remove_item->setString(1, item_name);
        remove_item->execute();
    }

    size_t GetItemAmmount(const std::string& item_name) {
        ThrowIfNoItem(item_name);
        sql::PreparedStatement* get_ammount = connection->prepareStatement("SELECT items_left FROM catalog WHERE name = (?)");
        get_ammount->setString(1, item_name);
        sql::ResultSet* res = get_ammount->executeQuery();
        if (res->next())
            return res->getInt(1);
    }

    size_t GetItemPrice(const std::string& item_name) {
        ThrowIfNoItem(item_name);
        sql::PreparedStatement* get_price = connection->prepareStatement("SELECT price FROM catalog WHERE name = (?)");
        get_price->setString(1, item_name);
        sql::ResultSet* res = get_price->executeQuery();
        if (res->next()) {
            return res->getInt(1);
        }
    }

    void ChangeItemPrice(const std::string& item_name, const size_t new_price) {
        ThrowIfNoItem(item_name);
        sql::PreparedStatement* change_price = connection->prepareStatement("UPDATE catalog SET price = (?) WHERE name = (?)");
        change_price->setInt(1, new_price);
        change_price->setString(2, item_name);
        change_price->execute();
    }

    // Clear functions made public just to test mysql_service with fixtures in googletest. In real usage they must be private
    void ClearAll() {

        ClearCatalog();
        ClearClients();
        ClearOrders();
    }

    void ClearCatalog() {
        sql::Statement* clear = connection->createStatement();
        clear->execute("TRUNCATE TABLE catalog");
    }

    void ClearClients() {
        sql::Statement* clear = connection->createStatement();
        clear->execute("TRUNCATE TABLE clients");
    }

    void ClearOrders() {
        sql::Statement* clear = connection->createStatement();
        clear->execute("TRUNCATE TABLE orders");
    }
    
    void ExitMySQL() {
        connection->close();
    }


private:

    // Check if item is available and we can remove 'change_item_amount' items from catalog
    void CheckDecreaseNumberOfItems(const std::string& item_name, const int& change_item_ammount) {
        
        size_t items_left;
        sql::PreparedStatement* stmt = connection->prepareStatement("SELECT items_left FROM catalog WHERE name = (?)");
        stmt->setString(1, item_name);
        sql::ResultSet* res = stmt->executeQuery();
        if (res->next()) {
            
            items_left = res->getInt(1);
            
            if (items_left < change_item_ammount) 
                throw std::invalid_argument("Invalid ammount of item " + item_name + " to remove!");
        
        } else {

            throw std::runtime_error("No item with id " + item_name + " in catalog!");    
            
        }
    }

    void ThrowIfItemExists(const std::string& item_name) {

        sql::PreparedStatement* check_exists = connection->prepareStatement("SELECT COUNT(name) FROM catalog WHERE name = (?)");
        check_exists->setString(1, item_name);
        sql::ResultSet* res = check_exists->executeQuery();
        if (res->next()) {
            if (res->getInt(1) != 0) throw std::runtime_error("Item " + item_name + " already exists in catalog!");
        }
    }

    void ThrowIfNoItem(const std::string& item_name) {

        sql::PreparedStatement* check_exists = connection->prepareStatement("SELECT COUNT(name) FROM catalog WHERE name = (?)");
        check_exists->setString(1, item_name);
        sql::ResultSet* res = check_exists->executeQuery();
        if (res->next()) {
            if (res->getInt(1) == 0) throw std::runtime_error("No item " + item_name + " in catalog!");
        }
    }

    void ThrowIfNoOrder(const size_t& order_id) {
        
        sql::PreparedStatement* check_exists = connection->prepareStatement("SELECT COUNT(id) FROM orders WHERE id = (?)");
        check_exists->setInt(1, order_id);
        sql::ResultSet* res = check_exists->executeQuery();
        if (res->next()) {
            if (res->getInt(1) == 0) throw std::runtime_error("No order with id " + std::to_string(order_id) + " in orders!");
        }
    }
   

    void ThrowIfClientExists(const std::string& client_mail) {
    
        sql::PreparedStatement* check_exists = connection->prepareStatement("SELECT COUNT(name) FROM clients WHERE mail = (?)");
        check_exists->setString(1, client_mail);
        sql::ResultSet* res = check_exists->executeQuery();
        if (res->next()) {
            if (res->getInt(1) != 0) throw std::runtime_error("Mail " + client_mail + " already in use!");
        }  
    }

    void ThrowIfNoClient(const std::string& client_mail) {
    
        sql::PreparedStatement* check_exists = connection->prepareStatement("SELECT COUNT(name) FROM clients WHERE mail = (?)");
        check_exists->setString(1, client_mail);
        sql::ResultSet* res = check_exists->executeQuery();
        if (res->next()) {
            if (res->getInt(1) == 0) throw std::runtime_error("No mail " + client_mail + " in 'clients'!");
        }  
    }
   

    void UpdateClientOrders(const std::string& client_mail, const size_t& total_order) {
        
        sql::PreparedStatement* update_client = connection->prepareStatement("UPDATE clients SET total_spent = total_spent + (?), "
                                                                             "total_orders = total_orders + 1 WHERE mail = (?)");
        update_client->setInt(1, total_order);
        update_client->setString(2, client_mail);
        update_client->execute();
    }


    void UpdateClientDiscount(const std::string& client_mail) {
       
        ThrowIfNoClient(client_mail);
        sql::PreparedStatement* get_total_spent = connection->prepareStatement("SELECT total_spent FROM clients WHERE mail = (?)");
        get_total_spent->setString(1, client_mail);
        sql::ResultSet* res_total = get_total_spent->executeQuery();
        size_t total_spent;
        if (res_total->next()) {
            total_spent = res_total->getInt(1);
        }

        if (total_spent >= 1000000) {
                
            sql::PreparedStatement* update_discount = connection->prepareStatement("UPDATE clients SET discount = (?) "
                                                                                   "WHERE mail = (?)");
            update_discount->setString(2, client_mail);
            
            if (total_spent >= 5000000) {
                
                update_discount->setInt(1, 25);
            
            } else if (total_spent >= 2500000) {
    
                update_discount->setInt(1, 15);

            } else if (total_spent >= 1000000) {

                update_discount->setInt(1, 5);

            }

            update_discount->execute();
        }
    }



    // Inits database 'store' and it's tables 'customers', 'orders' and 'catalog' if needed
    void CreateDatabase() {

        sql::Statement* stmt = connection->createStatement();
        stmt->execute("CREATE DATABASE IF NOT EXISTS store");
        connection->setSchema("store");
        
        stmt->execute("CREATE TABLE IF NOT EXISTS clients (id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(40),"
                      " mail VARCHAR(40), total_spent INT, total_orders INT, discount INT)");
        stmt->execute("ALTER TABLE clients AUTO_INCREMENT = 10000");

        stmt->execute("CREATE TABLE IF NOT EXISTS orders (id INT AUTO_INCREMENT PRIMARY KEY, client_mail VARCHAR(40),"
                      " data DATETIME, number_of_items INT, total_no_discount INT, total_with_discount INT)");
        stmt->execute("ALTER TABLE orders AUTO_INCREMENT = 100000");

        stmt->execute("CREATE TABLE IF NOT EXISTS catalog (id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(40),"
                      " price INT, items_left INT)");
        stmt->execute("ALTER TABLE catalog AUTO_INCREMENT = 10000");

    }


private:

	sql::Driver* driver = get_driver_instance();
	sql::Connection* connection;
