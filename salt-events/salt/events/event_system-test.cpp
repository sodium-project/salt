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

    Event_bus<Event<A>> bus;
    int                 a     = 0;
    int                 b     = 0;
    int                 count = 0;
    std::string         s;

    bus.attach_back<Event<A>>([&](Event<A>& e) {
        count++;
        a = e->a;
        e.consume();
        REQUIRE(e->a == 1);
    });

    bus.attach_back<Event<A>>([&](Event<A>&) {
        count++;
        REQUIRE_FALSE(true); // always false checking if event consuming is working right
    });

    bus.attach_front<Event<A>>([&](Event<A>& e) {
        count++;
        b = e->b;
        REQUIRE(e->b == 2);
        REQUIRE(a == 0);
    });

    bus.dispatch<Event<A>>(1, 2);
    REQUIRE(count == 2);
}

TEST_CASE("salt::Event_queue", "[salt-events/event_system.hpp]") {
    using namespace salt;

    Event_bus<Event<A>> bus;
    int                 a     = 0;
    int                 b     = 0;
    int                 count = 0;

    Event_queue<Event<A>, Event<B>> queue;
    bus.attach_back<Event<A>>([&](Event<A>& e) {
        count++;
        a = e->a;
        e.consume();
        REQUIRE(e->a == 1);
    });

    bus.attach_back<Event<A>>([&](Event<A>&) {
        count++;
        REQUIRE_FALSE(true); // always false checking if event consuming is working right
    });

    bus.attach_front<Event<A>>([&](Event<A>& e) {
        count++;
        b = e->b;
        REQUIRE(e->b == 2);
        REQUIRE(a == 0);
    });

    queue.push_back<Event<A>>(1, 2);

    REQUIRE(queue.size() == 1);
    REQUIRE_FALSE(queue.empty());

    bus.dispatch(queue);
    REQUIRE(queue.size() == 1);

    bus.dispatch(queue);
    REQUIRE(count == 2);
    REQUIRE(a == 1);

    queue.clear();
    REQUIRE(queue.size() == 0);
    REQUIRE(queue.empty());
}