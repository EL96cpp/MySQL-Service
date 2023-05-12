#pragma once

#include <string>

class Item {

public:
    
    Item(const std::string& name, const size_t& item_ammount) : name(name), item_ammount(item_ammount) {}

    Item(const std::string& name, const size_t& price, const size_t& item_ammount) : id(id), name(name),
		 							                                               price(price), 
                                                                                   item_ammount(item_ammount) {}


    Item(const size_t& id, const std::string& name,
		 const size_t& price, const size_t& item_ammount) : id(id), name(name),
		 												  price(price), item_ammount(item_ammount) {}
	
    size_t GetId() const { 
        return id; 
    }

    std::string GetName() const { 
        return name; 
    }
    
    size_t GetPrice() const {
        return price;
    }

    size_t GetAmmount() const {
        return item_ammount;
    }

private:
	size_t id;
	std::string name;
	size_t price;
	size_t item_ammount;
};
