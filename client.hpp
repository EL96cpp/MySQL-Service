#pragma once 

#include <string>


class Client {

public:
	Client(const std::string& name, const std::string& mail, const size_t& total_spent = 0,
           const size_t& number_of_orders = 0, const float& discount = 0.f) : name(name), mail(mail), 
                                                                              total_spent(total_spent), discount(discount) {}

	std::string GetName() const { 
        return name; 
    }

	std::string GetMail() const { 
        return mail; 
    }

	size_t GetTotalSpent() const { 
        return total_spent; 
    }	

    size_t GetNumberOfOrders() const {
        return number_of_orders;
    }
	
	float GetDiscount() const { 
        return discount; 
    }

private:
	std::string name;
	std::string mail;
	size_t total_spent = 0;
	size_t number_of_orders = 0;
    float discount = 0.f;
};
