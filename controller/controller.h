#include <drogon/HttpController.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/utils/coroutine.h>

class AuthController: public drogon::HttpController<AuthController>
{
private:

public:
   METHOD_LIST_BEGIN
   ADD_METHOD_TO(AuthController::sendCode, "/auth/send-code", drogon::Post);
   ADD_METHOD_TO(AuthController::verifyCode, "/auth/verify-code", drogon::Post);
   METHOD_LIST_END

   drogon::Task<drogon::HttpResponsePtr> sendCode(drogon::HttpRequestPtr req);
   drogon::Task<drogon::HttpResponsePtr> verifyCode(drogon::HttpRequestPtr req);
};
