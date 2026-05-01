#include "dotenv.h"
#include <drogon/HttpAppFramework.h>
#include <drogon/drogon.h>

int main()
{
   dotenv::init(dotenv::Preserve);

   drogon::app().loadConfigFile("./config.json");
   drogon::app().run();
}
