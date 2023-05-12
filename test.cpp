#include <vector>

#include <gtest/gtest.h>

#include "sql_service.hpp"
#include "item.hpp"
#include "client.hpp"
#include "order_data.hpp"

struct TestSqlService : public testing::Test {

    sql_service service;
    std::vector<Item> items;
    std::vector<Client> clients;

    TestSqlService() {}

    void SetUp() {
        service.Connect("127.0.0.1", 3306);
        Item item1("Keyboard1", 45000, 10), item2("Keyboard2", 75000, 15), item3("Keyboard3", 100000, 10), 
             item4("Mouse1", 125000, 5), item5("Mouse2", 110000, 10), item6("Mouse3", 758000, 10),
             item7("CPU1", 220000, 10), item8("CPU2", 2355000, 11), item9("CPU3", 7599000, 5);
        Client client1("name1", "mail1@gmail.com"), client2("name2", "mail2@gmail.com"), client3("name3", "mail3@gmail.com");
        items.insert(items.end(), {item1, item2, item3, item4, item5, item6, item7, item8, item9});
        clients.insert(clients.end(), {client1, client2, client3});
        for (auto& item : items) {
            service.AddToCatalog(item);
        }
        for (auto& client : clients) {
            service.AddClient(client);
        }
    }
    
    void TearDown() {
        service.ClearAll();
        service.ExitMySQL();
    }

};


// Test adding clients and items to database
TEST_F(TestSqlService, TestAdd) {
    for (auto& client : clients) {
        EXPECT_EQ(client.GetName(), service.GetClientName(client.GetMail()));
        EXPECT_EQ(0, service.GetClientTotalSpent(client.GetMail()));
        EXPECT_EQ(0, service.GetClientTotalOrders(client.GetMail()));
        EXPECT_EQ(0, service.GetClientDiscount(client.GetMail()));
    }

    for (auto& item : items) {
       EXPECT_EQ(item.GetPrice(), service.GetItemPrice(item.GetName()));
       EXPECT_EQ(item.GetAmmount(), service.GetItemAmmount(item.GetName()));
    }
}

// Test, if service correctly removes clients/items and throws exception when we ask for removed client/item 
TEST_F(TestSqlService, TestRemove) {
    for (auto& client : clients) {
        service.RemoveClient(client.GetMail());
        EXPECT_THROW(service.GetClientName(client.GetMail()), std::runtime_error);
    }
    for (auto& item : items) {
        service.RemoveItem(item.GetName());
        EXPECT_THROW(service.GetItemAmmount(item.GetName()), std::runtime_error);
    }
}


TEST_F(TestSqlService, TestDecreaseNumberOfItems) {
    for (auto& item : items) {
        service.DecreaseNumberOfItems(item.GetName(), 5);
    }
    for (int i = 0; i < items.size(); ++i) {
        EXPECT_EQ(items[i].GetAmmount() - 5, service.GetItemAmmount(items[i].GetName()));
    }
}

TEST_F(TestSqlService, TestIncreaseNumberOfItems) {
    for (auto& item : items) {
        service.IncreaseNumberOfItems(item.GetName(), 15);
    }
    for (int i = 0; i < items.size(); ++i) {
        EXPECT_EQ(items[i].GetAmmount() + 15, service.GetItemAmmount(items[i].GetName()));
    }
}

TEST_F(TestSqlService, TestIncreaseNumberOfItemsZero) {
    for (auto& item : items) {
        service.IncreaseNumberOfItems(item.GetName(), 0);
    }
    for (auto& item : items) {
        EXPECT_EQ(item.GetAmmount(), service.GetItemAmmount(item.GetName()));
    }
}

TEST_F(TestSqlService, TestDecreaseNumberOfItemsZero) {
    for (auto& item : items) {
        service.DecreaseNumberOfItems(item.GetName(), 0);
    }
    for (auto& item : items) {
        EXPECT_EQ(item.GetAmmount(), service.GetItemAmmount(item.GetName()));
    }
}


TEST_F(TestSqlService, TestOrder) {
    
    Item item1("Mouse1", 125000, 1), item2("Keyboard1", 45000, 1), item3("CPU1", 220000, 1);
    std::vector<Item> order = {item1, item2, item3};
    std::vector<size_t> ammounts = GetAmmounts(order, service);
    OrderData order_data = GetOrderData(clients[0].GetMail(), order, service);
    service.MakeOrder(clients[0].GetMail(), order);

    EXPECT_EQ(0, service.GetClientDiscount(clients[0].GetMail()));    
    EXPECT_EQ(1, service.GetClientTotalOrders(clients[0].GetMail()));
    EXPECT_EQ(order_data.total_with_discount, service.GetClientTotalSpent(clients[0].GetMail()));
    EXPECT_EQ(order_data.total_no_discount, service.GetOrderTotalNoDiscount(100000));
    EXPECT_EQ(order_data.total_with_discount, service.GetOrderTotalWithDiscount(100000));
    EXPECT_EQ(order_data.number_of_items, service.GetOrderNumberOfItems(100000));
    for (int i = 0; i < order.size(); ++i) {
        EXPECT_EQ(ammounts[i] - order[i].GetAmmount(), service.GetItemAmmount(order[i].GetName()));
    }
}

TEST_F(TestSqlService, TestOrderDiscount) {

    // First order, makes discount equal to 5%
    Item item1("Mouse3", 758000, 2), item2("CPU1", 220000, 2);
    std::vector<Item> order1 = {item1, item2};
    std::vector<size_t> ammounts1 = GetAmmounts(order1, service);
    OrderData order_data1 = GetOrderData(clients[0].GetMail(), order1, service);   
    service.MakeOrder(clients[0].GetMail(), order1);
    
    EXPECT_EQ(5, service.GetClientDiscount(clients[0].GetMail()));    
    EXPECT_EQ(1, service.GetClientTotalOrders(clients[0].GetMail()));
    EXPECT_EQ(order_data1.total_with_discount, service.GetClientTotalSpent(clients[0].GetMail()));
    EXPECT_EQ(order_data1.total_no_discount, service.GetOrderTotalNoDiscount(100000));
    EXPECT_EQ(order_data1.total_with_discount, service.GetOrderTotalWithDiscount(100000));
    EXPECT_EQ(order_data1.number_of_items, service.GetOrderNumberOfItems(100000));
    for (int i = 0; i < order1.size(); ++i) {
        EXPECT_EQ(ammounts1[i] - order1[i].GetAmmount(), service.GetItemAmmount(order1[i].GetName()));
    }

    // Second order, makes discount 15%
    Item item3("Mouse3", 758000, 1), item4("CPU1", 220000, 1);
    std::vector<Item> order2 = {item3, item4};
    std::vector<size_t> ammounts2 = GetAmmounts(order2, service);
    OrderData order_data2 = GetOrderData(clients[0].GetMail(), order2, service);
    service.MakeOrder(clients[0].GetMail(), order2);

    EXPECT_EQ(15, service.GetClientDiscount(clients[0].GetMail()));    
    EXPECT_EQ(2, service.GetClientTotalOrders(clients[0].GetMail()));
    EXPECT_EQ(order_data2.total_with_discount + order_data1.total_with_discount, service.GetClientTotalSpent(clients[0].GetMail()));
    EXPECT_EQ(order_data2.total_no_discount, service.GetOrderTotalNoDiscount(100001));
    EXPECT_EQ(order_data2.total_with_discount, service.GetOrderTotalWithDiscount(100001));
    EXPECT_EQ(order_data2.number_of_items, service.GetOrderNumberOfItems(100001));
    for (int i = 0; i < order2.size(); ++i) {
        EXPECT_EQ(ammounts2[i] - order2[i].GetAmmount(), service.GetItemAmmount(order2[i].GetName()));
    }

    // Thrid order, makes discount 25%, we make it for clients[0] and clients[2] to check if discount works
    Item item5("CPU3", 7599000, 1);
    std::vector<Item> order3 = { item5 };
    std::vector<size_t> ammounts3 = GetAmmounts(order3, service);
    OrderData order_data3 = GetOrderData(clients[0].GetMail(), order3, service); 
    service.MakeOrder(clients[0].GetMail(), order3);
    OrderData order_data4 = GetOrderData(clients[2].GetMail(), order3, service);
    service.MakeOrder(clients[2].GetMail(), order3);

    EXPECT_EQ(25, service.GetClientDiscount(clients[0].GetMail()));
    EXPECT_EQ(25, service.GetClientDiscount(clients[2].GetMail()));
    EXPECT_EQ(3, service.GetClientTotalOrders(clients[0].GetMail()));
    EXPECT_EQ(1, service.GetClientTotalOrders(clients[2].GetMail()));

    EXPECT_EQ(order_data3.total_with_discount + order_data2.total_with_discount + 
              order_data1.total_with_discount, service.GetClientTotalSpent(clients[0].GetMail()));
    EXPECT_EQ(order_data3.total_no_discount, service.GetOrderTotalNoDiscount(100002));
    EXPECT_EQ(order_data3.total_with_discount, service.GetOrderTotalWithDiscount(100002));
    EXPECT_EQ(order_data3.number_of_items, service.GetOrderNumberOfItems(100002));

    EXPECT_EQ(order_data4.total_no_discount, service.GetOrderTotalNoDiscount(100003));
    EXPECT_EQ(order_data4.total_with_discount, service.GetOrderTotalWithDiscount(100003));
    EXPECT_EQ(order_data4.number_of_items, service.GetOrderNumberOfItems(100003));

    EXPECT_EQ(ammounts3[0] - 2, service.GetItemAmmount(item5.GetName()));

}

