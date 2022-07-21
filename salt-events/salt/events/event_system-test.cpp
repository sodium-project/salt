#include <catch2/catch.hpp>

#include <salt/events.hpp>

struct A {
    int a;
    int b;
};

struct B {
    char s;
};

TEST_CASE("salt::Event_bus", "[salt-events/event_system.hpp]") {
    using namespace salt;

    SECTION("test attach") {
        Event_bus<Event<A>> bus;
        int                 a     = 0;
        int                 b     = 0;
        int                 count = 0;
        std::string         s;

        bus.attach<Event<A>>([&](Event<A>& e) {
            count++;
            a = e->a;
            b = e->b;
            e.consume();
            REQUIRE(a == 1);
            REQUIRE(b == 2);
        });

        bus.attach<Event<A>>([&](Event<A>&) {
            count++;
            REQUIRE_FALSE(true); // always false checking if event consuming is working right
        });

        bus.dispatch<Event<A>>(1, 2);
        REQUIRE(count == 1);
    }

    SECTION("test detach") {
        Event_bus<Event<A>> bus;
        int                 a     = 0;
        int                 b     = 0;
        int                 count = 0;
        std::string         s;

        auto callback_key = bus.attach<Event<A>>([&](Event<A>& e) {
            count++;
            a = e->a;
            b = e->b;
            e.consume();
            REQUIRE(a == 1);
            REQUIRE(b == 2);
        });

        bus.dispatch<Event<A>>(1, 2);
        REQUIRE(count == 1);

        bus.detach<Event<A>>(callback_key);
        bus.dispatch<Event<A>>(1, 2);
        REQUIRE(count == 1);
    }
}

TEST_CASE("salt::Event_queue", "[salt-events/event_system.hpp]") {
    using namespace salt;
    SECTION("test attach") {
        Event_bus<Event<A>> bus;
        int                 a     = 0;
        int                 b     = 0;
        int                 count = 0;

        Event_queue<Event<A>, Event<B>> queue;
        bus.attach<Event<A>>([&](Event<A>& e) {
            count++;
            a = e->a;
            b = e->b;
            e.consume();
            REQUIRE(a == 1);
            REQUIRE(b == 2);
        });

        bus.attach<Event<A>>([&](Event<A>&) {
            count++;
            REQUIRE_FALSE(true); // always false checking if event consuming is working right
        });

        queue.push_back<Event<A>>(1, 2);

        REQUIRE(queue.size() == 1);
        REQUIRE_FALSE(queue.empty());

        bus.dispatch(queue);
        REQUIRE(queue.size() == 1);

        bus.dispatch(queue);
        REQUIRE(count == 1);
        REQUIRE(a == 1);

        queue.clear();
        REQUIRE(queue.size() == 0);
        REQUIRE(queue.empty());
    }

    SECTION("test move") {
        Event_queue<Event<A>> q1;

        q1.push_back<Event<A>>(1, 2);
        REQUIRE(q1.size() == 1);
        REQUIRE_FALSE(q1.empty());

        Event_queue<Event<A>> q2{std::move(q1)};
        REQUIRE(q2.size() == 1);
        REQUIRE_FALSE(q2.empty());

        Event_queue<Event<A>> q3;
        q3 = std::move(q2);
        REQUIRE(q3.size() == 1);
        REQUIRE_FALSE(q3.empty());

        Event_bus<Event<A>> bus;
        int                 count = 0;
        bus.attach<Event<A>>([&](Event<A>&) {
            count++;
        });

        bus.dispatch(q3);
        REQUIRE(count == 1);

        q3.clear();
        REQUIRE(q3.size() == 0);
        REQUIRE(q3.empty());
    }
}