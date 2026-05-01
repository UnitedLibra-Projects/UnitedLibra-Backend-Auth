#include <drogon/HttpController.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/utils/coroutine.h>
#include <string>

inline const std::string HOST_ADDRESS = "http://127.0.0.1:8000";

class EmailVerificationController: public drogon::HttpController<EmailVerificationController>
{
private:

public:
   METHOD_LIST_BEGIN
   ADD_METHOD_TO(EmailVerificationController::sendCode, "/auth/send-code", drogon::Post);
   ADD_METHOD_TO(EmailVerificationController::verifyCode, "/auth/verify-code", drogon::Post);
   METHOD_LIST_END

   drogon::Task<drogon::HttpResponsePtr> sendCode(drogon::HttpRequestPtr req);
   drogon::Task<drogon::HttpResponsePtr> verifyCode(drogon::HttpRequestPtr req);
};

class UserActionController: public drogon::HttpController<UserActionController>
{
public:
   METHOD_LIST_BEGIN
   ADD_METHOD_TO(UserActionController::findUser, "/auth/find-user", drogon::Post);
   ADD_METHOD_TO(UserActionController::addUser, "/auth/add-user", drogon::Post);
   METHOD_LIST_END

   drogon::Task<drogon::HttpResponsePtr> findUser(drogon::HttpRequestPtr req);
   drogon::Task<drogon::HttpResponsePtr> addUser(drogon::HttpRequestPtr req);
};

class AuthController: public drogon::HttpController<AuthController>
{
public:
   METHOD_LIST_BEGIN
   ADD_METHOD_TO(AuthController::loginUser, "/auth/login-user", drogon::Post);
   ADD_METHOD_TO(AuthController::registerUser, "auth/register-user", drogon::Post);
   METHOD_LIST_END

   drogon::Task<drogon::HttpResponsePtr> loginUser(drogon::HttpRequestPtr req);
   drogon::Task<drogon::HttpResponsePtr> registerUser(drogon::HttpRequestPtr req);
};
