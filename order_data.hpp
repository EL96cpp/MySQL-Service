#pragma once

#include <vector>

#include "sql_service.hpp"
#include "item.hpp"

struct OrderData {
    
    OrderData(const size_t& number_of_items, const size_t& total_no_discount, const size_t& total_with_discount) : 
              number_of_items(number_of_items), total_no_discount(total_no_discount), 
              total_with_discount(total_with_discount) {}
    
    size_t number_of_items;
    size_t total_no_discount;
    size_t total_with_discount;
};

OrderData GetOrderData(const std::string& client_mail, const std::vector<Item>& order, sql_service& service) {
    size_t discount = service.GetClientDiscount(client_mail);
    size_t number_of_items = 0, total_no_discount = 0;
    for (auto& item : order) {
        number_of_items += item.GetAmmount();
        total_no_discount += item.GetPrice() * item.GetAmmount();
    }
    size_t total_with_discount = total_no_discount - total_no_discount * discount/100;
    return OrderData(number_of_items, total_no_discount, total_with_discount);
}

std::vector<size_t> GetAmmounts(const std::vector<Item>& order, sql_service& service) {
    std::vector<size_t> ammounts;
    for (auto& item : order) {
        ammounts.push_back(service.GetItemAmmount(item.GetName()));
    }
    return ammounts;
}
