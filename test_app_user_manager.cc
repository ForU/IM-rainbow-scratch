#include "user_manager.hh"

void test_adduser(UserManager &manager, const char *id)
{
}

int main(int argc, char *argv[])
{
    UserManager um;

    UserId user_id;

    um.newUser(user_id, "xiangshou24", "hello everyone", "xiangshou24@gmail.com");
    LOG_INFO << "user id=" << user_id.id();

    um.newUser(user_id, "lees", "hi", "tomi@soft.com");
    LOG_INFO << "user id=" << user_id.id();

    um.newUser(user_id, "jacoo", "yolo");
    LOG_INFO << "user id=" << user_id.id();

    um.showUser(UserId((unsigned long)0));

    um.makeFriends(UserId("1"), UserId("2"));
    um.makeFriends(UserId("1"), UserId("23"));

    for ( int i = 1; i <4; ++i ) {
        um.showUser(UserId(i));
    }

    return 0;
}
