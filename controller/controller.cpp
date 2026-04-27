#include "controller.h"
#include "dotenv.h"
#include <cstdlib>
#include <drogon/HttpClient.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <random>
#include <chrono>
#include <drogon/HttpController.h>
#include <drogon/utils/coroutine.h>
#include <string>
#include <unordered_map>

struct EmailCode
{
   std::string code;
   std::chrono::steady_clock::time_point expiresAt;
};

std::unordered_map<std::string, EmailCode> codes;

std::string generateCode() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(100000, 999999);
    return std::to_string(dist(rng));
}

void sendEmailCode(std::string email, std::string code)
{
   auto client = drogon::HttpClient::newHttpClient("https://api.resend.com");


    Json::Value body;
    body["from"] = "UnitedLibra <onboarding@resend.dev>";
    body["to"] = Json::arrayValue;
    body["to"].append(email);
    body["subject"] = "Код входа";
    body["text"] = "Ваш код: " + code;

    auto req = drogon::HttpRequest::newHttpJsonRequest(body);
    req->setMethod(drogon::Post);
    req->setPath("/emails");
    req->addHeader("Authorization", dotenv::getenv("API_EMAIL_SENDER"));

    client->sendRequest(req, [](drogon::ReqResult result, const drogon::HttpResponsePtr &resp) {
        if (result != drogon::ReqResult::Ok) {
            LOG_ERROR << "Email send failed";
            return;
        }

        LOG_INFO << "Email sent: " << resp->getBody();
    });
}

drogon::Task<drogon::HttpResponsePtr> AuthController::sendCode(drogon::HttpRequestPtr req)
{
   auto email_json = req->getJsonObject();

   if(!email_json || !email_json->isMember("email"))
   {
      co_return drogon::HttpResponse::newHttpJsonResponse(Json::Value("invalide phone number"));
   }

   std::string email = (*email_json)["email"].asString();
   std::string code = generateCode();

   EmailCode data;
   data.code = code;
   data.expiresAt = std::chrono::steady_clock::now() + std::chrono::minutes(5);

   codes[email] = data;

   sendEmailCode(email, code);

   Json::Value status;
   status["status"] = "ok";

   auto resp = drogon::HttpResponse::newHttpJsonResponse(status);
   resp->setStatusCode(drogon::k200OK);

   co_return resp;
}

drogon::Task<drogon::HttpResponsePtr> AuthController::verifyCode(drogon::HttpRequestPtr req)
{
   co_return drogon::HttpResponse::newHttpResponse();
}
