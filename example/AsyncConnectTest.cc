#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <thread>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    auto reply = static_cast<redisReply*>(r);
    if (reply == NULL) {
        if (c->errstr) {
            printf("errstr: %s\n", c->errstr);
        }
        return;
    }
    printf("key: %s, type: %d\n", reply->str, reply->type);

    // redisAsyncDisconnect(c);
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
}

int main (int argc, char **argv) {
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif

    struct event_base *base = event_base_new();
    redisOptions options = {0};
    REDIS_OPTIONS_SET_TCP(&options, "127.0.0.1", 6379);
    struct timeval tv = {0};
    tv.tv_sec = 1;
    options.connect_timeout = &tv;


    redisAsyncContext *c = redisAsyncConnectWithOptions(&options);
    if (c->err) {
        /* Let *c leak for now... */
        printf("Error: %s\n", c->errstr);
        return 1;
    }

    redisLibeventAttach(c,base);
    redisAsyncSetConnectCallback(c,connectCallback);
    redisAsyncSetDisconnectCallback(c,disconnectCallback);
    auto t = new std::thread([&](){
        event_base_dispatch(base);
    });


    for (int j = 0; j < 10; ++j) {
        redisAsyncCommand(c, NULL, NULL, "SET key %d", j);
        redisAsyncCommand(c, getCallback, NULL, "GET key");
    }

    redisAsyncDisconnect(c);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
