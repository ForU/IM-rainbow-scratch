// message
// 	  -> request
// 	  -> response
// 	  -> notification

////////////////////////////////////////////////////////////////
// 1. packet get "type" string and "body" string, packet has two method:
//    encapsulate() and extract()
// 2. "body" string is parsed by different message type
// 3. every detail-message has two method:
//    parse() and serialize()

// packet error
{
    "type":"fatal",
    "body": {
        "fatal_info":""
    }
}
////////////////////////////////////////////////////////////////
// register request
{
    "type":"register-request",
    "body": {
        "name":_string_,
        "email":_string_,
        "password":_string_,
        "signature":_string_
    }
}
////////////////////////////////////////////////////////////////
// login request
{
    "type":"login-request",     // login request
    "body": {
        "user_id":"00002",
        "login_value":"",
        "password":"",
        "extra_info":"this can be emtpy"
    },
}
////////////////////////////////////////////////////////////////
// login response
{

    "type": "login-response",
    "body":{
        "error_code":1,
        "error_info": "invalid login name or password",
        "extra_info":"this can be emtpy"
    }
}
////////////////////////////////////////////////////////////////
// login success response
{
    "type": "login-response",
    "body": {
        "error_code":0,
        "friends": [
            {"user_id":*, "register_name":*, "signature":*, "status":*},
            {"user_id":*, "register_name":*, "signature":*, "status":*},
            {"user_id":*, "register_name":*, "signature":*, "status":*}
        ],
        "groups": [ ],
        "status":*,
        "register_name":*,
        "signature":*,
        "user_id":*,
        "identification": "jdf89adf98djoeuroe8r09082334jl"
        "extra_info": "this maybe empty, happy hacking"
    }
}
////////////////////////////////////////////////////////////////
// chat: A-B, B-A same
{
    "type":5,
    "body": {
        "from":"00002", "to":"00001",
        "data": "how is it going, xxxx",
        "time":"seconds",
        "extra_info":""
    }
}
////////////////////////////////////////////////////////////////
{
    "type": "login_status",
    "body": {
        "from": "00001",
        "status": "off-line"    // "on-line"
    }
}

////////////////////////////////////////////////////////////////

// {
//     "type": "login",
//     "body": {
//         "from": "00001"
//     }
// }

////////////////////////////////////////////////////////////////
{
    "type": "update signature",
    "body": {
        "from": "00001",
        "signature": "some text here"
    }
}
////////////////////////////////////////////////////////////////


// add Friends
// delete Friends
// update Friends category

{
    "type": 9,
    "body": {
        "type": "add",          // delete, update
        "requester_id": "00001",
        "target_id": "00002",
        "category": "Friends"
    }
}

// add response
{
    "type": "Friends management add response",
    "body": {
        "target_user_id": "00002",
        "target_name":"jkjdf",
        "target_signature": "xxxxx",
        "target_status":"off-line",
        "category": ""
    }
}

