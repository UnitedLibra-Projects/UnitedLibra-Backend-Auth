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
   ADD_METHOD_TO(AuthController::RegisterUser, "/register", drogon::Post);
   ADD_METHOD_TO(AuthController::LoginUser, "/login", drogon::Post);
   METHOD_LIST_END

   drogon::Task<drogon::HttpResponsePtr> RegisterUser(drogon::HttpRequestPtr req);
   drogon::Task<drogon::HttpResponsePtr> LoginUser(drogon::HttpRequestPtr req);
};
