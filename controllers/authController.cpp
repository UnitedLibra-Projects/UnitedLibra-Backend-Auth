#include "controller.h"
#include <drogon/HttpAppFramework.h>
#include <drogon/HttpClient.h>
#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/utils/coroutine.h>
#include <exception>
#include <string>
#include <trantor/utils/Logger.h>
#include <openssl/evp.h>

std::string simple_sha256(const std::string& input) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, input.c_str(), input.size());
    EVP_DigestFinal_ex(ctx, hash, &hash_len);
    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < hash_len; ++i)
        oss << std::setw(2) << static_cast<int>(hash[i]);
    return oss.str();
}

drogon::Task<drogon::HttpResponsePtr> AuthController::loginUser(drogon::HttpRequestPtr req)
{  
   auto req_json = req->getJsonObject();

   std::string email = (*req_json)["email"].asString();
   std::string password = (*req_json)["password"].asString();
   password = simple_sha256(password);

   auto findUser_client = drogon::HttpClient::newHttpClient(HOST_ADDRESS);

   Json::Value findUser_json;
   findUser_json["email"] = email;
   
   auto findUser_req = drogon::HttpRequest::newHttpJsonRequest(findUser_json);
   findUser_req->setMethod(drogon::Post);
   findUser_req->setPath("/auth/find-user");

   drogon::HttpResponsePtr findUser_resp;
   try {
      findUser_resp = co_await findUser_client->sendRequestCoro(findUser_req);

//      LOG_INFO << 
   } catch (std::exception& ex) {
      LOG_ERROR << "error: " << ex.what();
   }

   drogon::HttpResponsePtr resp;
   Json::Value resp_json;

   auto statusCode_resp = findUser_resp->getStatusCode();
   
   if(statusCode_resp == drogon::k404NotFound)
   {
      resp_json["error"] = "user not found";
      resp = drogon::HttpResponse::newHttpJsonResponse(resp_json);
      resp->setStatusCode(drogon::k404NotFound);

      co_return resp;
   }

   auto db_client = drogon::app().getDbClient();

   auto db_password = co_await db_client->execSqlCoro(
         R"(
         SELECT password
         FROM users
         WHERE email = $1;
         )",
         email
         ); 
   if(password != db_password[0]["password"].as<std::string>())
   {
      resp_json["error"] = "incorrect password";
      resp = drogon::HttpResponse::newHttpJsonResponse(resp_json);
      resp->setStatusCode(drogon::k401Unauthorized);

      co_return resp;
   }

   auto sendCode_client = drogon::HttpClient::newHttpClient(HOST_ADDRESS);

   Json::Value sendCode_json;
   sendCode_json["email"] = email;

   auto sendCode_req = drogon::HttpRequest::newHttpJsonRequest(sendCode_json);
   sendCode_req->setMethod(drogon::Post);
   sendCode_req->setPath("/auth/send-code");

   try {
      co_await sendCode_client->sendRequestCoro(sendCode_req);
   } catch (std::exception& ex) {
      LOG_ERROR << "error: " << ex.what();
   }

   resp_json["status"] = "ok";
   resp = drogon::HttpResponse::newHttpJsonResponse(resp_json);
   resp->setStatusCode(drogon::k200OK);

   co_return resp;
}

drogon::Task<drogon::HttpResponsePtr> AuthController::registerUser(drogon::HttpRequestPtr req)
{
   auto req_json = req->getJsonObject(); 

   LOG_INFO << "Content-Type: " << req->getHeader("content-type");

   if (!req_json) {
    LOG_ERROR << "JSON is null! Body length: " << req->bodyLength();
}

   std::string name = (*req_json)["name"].asString();
   std::string email = (*req_json)["email"].asString();
   std::string password = (*req_json)["password"].asString();
   password = simple_sha256(password);
   bool is_verifyCode = (*req_json)["is_verifyCode"].asBool();

   LOG_INFO << "is_verifyCode value: " << (is_verifyCode ? "true" : "false");

   Json::Value resp_json;
   drogon::HttpResponsePtr resp;

   if(is_verifyCode)
   {
      LOG_INFO << "Entering addUser block...";
      auto addUser_client = drogon::HttpClient::newHttpClient(HOST_ADDRESS);

      Json::Value addUser_json;
      addUser_json["name"] = name;
      addUser_json["email"] = email;
      addUser_json["password"] = password;

      auto addUser_req = drogon::HttpRequest::newHttpJsonRequest(addUser_json);
      addUser_req->setMethod(drogon::Post);
      addUser_req->setPath("/auth/add-user");

      try {
         co_await addUser_client->sendRequestCoro(addUser_req);
      } catch (std::exception& ex) {
         LOG_ERROR << "error: " << ex.what(); 
      }

      resp_json["status"] = "ok";

      resp = drogon::HttpResponse::newHttpJsonResponse(resp_json);

      co_return resp;
   }

   auto findUser_client = drogon::HttpClient::newHttpClient(HOST_ADDRESS);

   Json::Value findUser_json;
   findUser_json["name"] = name;
   findUser_json["email"] = email;

   auto findUser_req = drogon::HttpRequest::newHttpJsonRequest(findUser_json);
   findUser_req->setMethod(drogon::Post);
   findUser_req->setPath("/auth/find-user");
   
   drogon::HttpResponsePtr findUser_resp;
   try {
      findUser_resp = co_await findUser_client->sendRequestCoro(findUser_req);

      if (!findUser_resp) {
        LOG_ERROR << "Failed to get response from find-user service";
    }

   } catch (std::exception& ex) {
      LOG_ERROR << "error: " << ex.what(); 

      co_return drogon::HttpResponse::newNotFoundResponse();
   }

   if (!findUser_resp) {
      LOG_ERROR << "Request finished without exception, but response is NULL (nullptr)";

      auto resp = drogon::HttpResponse::newHttpResponse();
      resp->setStatusCode(drogon::k500InternalServerError);
      resp->setBody("External service did not return a response object");
      co_return resp;
   }

   auto statusCode_resp = findUser_resp->getStatusCode();

   if(statusCode_resp != drogon::k404NotFound)
   {
      resp_json["error"] = "user already exists";
      resp = drogon::HttpResponse::newHttpJsonResponse(resp_json);

      co_return resp;
   }
   
   auto sendCode_client = drogon::HttpClient::newHttpClient(HOST_ADDRESS);

   Json::Value sendCode_json;
   sendCode_json["email"] = email;

   auto sendCode_req = drogon::HttpRequest::newHttpJsonRequest(sendCode_json);
   sendCode_req->setMethod(drogon::Post);
   sendCode_req->setPath("/auth/send-code");

   try {
      co_await sendCode_client->sendRequestCoro(sendCode_req);
   } catch (std::exception& ex) {
      LOG_ERROR << "error: " << ex.what();
   }

   resp_json["status"] = "ok";
   resp = drogon::HttpResponse::newHttpJsonResponse(resp_json);

   co_return resp;
}
