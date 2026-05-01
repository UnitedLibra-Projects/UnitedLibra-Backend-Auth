#include "controller.h"
#include <drogon/HttpAppFramework.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>
#include <drogon/utils/coroutine.h>
#include <exception>
#include <optional>
#include <string>

drogon::Task<drogon::HttpResponsePtr> UserActionController::findUser(drogon::HttpRequestPtr req)
{
   auto user_json = req->getJsonObject();

   if (!user_json) {
    auto resp = drogon::HttpResponse::newHttpResponse();
    resp->setStatusCode(drogon::k400BadRequest);
    co_return resp;
   }

   std::string name;
   std::string email = (*user_json)["email"].asString();
   if(user_json->isMember("name"))
   {
      name = (*user_json)["name"].asString();
   }

   auto dbClient = drogon::app().getDbClient();

   std::optional<drogon::orm::Result> name_db;
   try {
      name_db = co_await dbClient->execSqlCoro(
            R"(
            SELECT name 
            FROM users
            WHERE email = $1;
            )",
            email
            );
   }
   catch(std::exception& ex)
   {
      LOG_ERROR << "error: " << ex.what();
   }

   Json::Value error;

   if(name_db->empty())
   {
      error["error"] = "user not found";
      
      auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
      resp->setStatusCode(drogon::k404NotFound);

      co_return resp;
   }
   else 
   {         
      Json::Value status;
      status["status"] = "ok";

      auto resp = drogon::HttpResponse::newHttpJsonResponse(status);
      resp->setStatusCode(drogon::k200OK);

      co_return resp;    
   }


   if(name != (*name_db)[0]["name"].as<std::string>())
   {
      error["error"] = "user already exists";

      auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
      resp->setStatusCode(drogon::k409Conflict);

      co_return resp;
   }
}

drogon::Task<drogon::HttpResponsePtr> UserActionController::addUser(drogon::HttpRequestPtr req)
{
   auto req_json = req->getJsonObject();

   if (!req_json) {
        LOG_ERROR << "addUser: JSON is null! Body: " << req->getBody();
        auto res = drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k400BadRequest);
        co_return res;
    }

   std::string name = (*req_json)["name"].asString();
   std::string email = (*req_json)["email"].asString();
   std::string password = (*req_json)["password"].asString();

   auto db_client = drogon::app().getDbClient();

   try {
      co_await db_client->execSqlCoro(
            R"(
            INSERT INTO users (name, email, password)
            VALUES ($1, $2, $3)
            )",
            name, email, password
            );
   } catch (const std::exception &e) {
        LOG_ERROR << "DB Error in addUser: " << e.what();
        auto res = drogon::HttpResponse::newHttpResponse();
        res->setStatusCode(drogon::k500InternalServerError);
        co_return res;
    }

   Json::Value resp_json;
   resp_json["status"] = "ok";
   auto resp = drogon::HttpResponse::newHttpJsonResponse(resp_json);

   LOG_INFO << "!!! ADD_USER FINISHED SUCCESSFULLY FOR: " << email;

   co_return resp;
}
