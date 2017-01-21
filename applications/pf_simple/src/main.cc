/**
 * GLOBALS["default.engine.frame"] = number;      //default 100.
 * GLOBALS["default.net.open"] = bool;            //default false.
 * GLOBALS["default.net.service"] = bool;         //default false.
 * GLOBALS["default.net.service_ip"] = string;    //default "".
 * GLOBALS["default.net.service_port"] = number;  //default 0.
 * GLOBALS["default.net.conn_max"] = number;      //default NET_CONNECTION_MAX.
 * GLOBALS["default.script.open"] = bool;         //default false.
 * GLOBALS["default.script.rootpath"] = string;   //default SCRIPT_ROOT_PATH.
 * GLOBALS["default.script.workpath"] = string;   //default SCRIPT_WORK_PATH.
 * GLOBALS["default.script.bootstrap"] = string;  //default "bootstrap.lua".
 * GLOBALS["default.script.type"] = number;       //default pf_script::kTypeLua.
 * GLOBALS["default.cache.open"] = bool;          //default fasle.
 * GLOBALS["default.cache.service"] = bool;       //default fasle.
 * GLOBALS["default.cache.conf"] = string;        //default "".
 * GLOBALS["default.cache.key_map"] = number;     //default ID_INVALID.
 * GLOBALS["default.cache.recycle_map"] = number; //default ID_INVALID.
 * GLOBALS["default.cache.query_map"] = number;   //default ID_INVALID.
 * GLOBALS["default.db.open"] = bool;             //default fasle.
 * GLOBALS["default.db.type"] = number;           //default kDBConnectorTypeODBC.
 * GLOBALS["default.db.server"] = string;         //default "".
 * GLOBALS["default.db.user"] = string;           //default "".
 * GLOBALS["default.db.password"] = string;       //default "".
 **/

#include "main.h"
#include "net.h"
#include "packet/sayhello.h"

//The script reload function.
void reload() {
  if (is_null(ENGINE_POINTER)) return;
  auto env = ENGINE_POINTER->get_script();
  if (is_null(env)) return;
  env->reload("preload.lua");
}

//The test engine main loop event 1.
int32_t times = 0;
void main_loop(pf_engine::Kernel &engine) {
  std::cout << "main_loop ..." << std::endl;
  ++times;
  if (times > 10)
    std::cout << "main_loop exited by 10 times" << std::endl;
  else
    engine.enqueue([&engine](){ main_loop(engine); });
}

//The test engine main loop event 2.
void main_loop1(pf_engine::Kernel &engine) {
  std::cout << "main_loop1 ..." << std::endl;
  ++times;
  if (times > 20)
    std::cout << "main_loop1 exited by 20 times" << std::endl;
  else
    engine.enqueue([&engine](){ main_loop1(engine); });
}

//Net test.
pf_net::connection::Basic *connector{nullptr};
void main_nconnect(pf_engine::Kernel &engine,
                   pf_net::connection::manager::Connector &mconnector) {
  mconnector.tick();
  if (is_null(connector)) {
    connector = mconnector.connect(
        "127.0.0.1", GLOBALS["default.net.service_port"].uint16());
  } else {
    static uint32_t last_time = 0;
    auto tickcount = TIME_MANAGER_POINTER->get_tickcount();
    if (tickcount - last_time > 5000) {
      SayHello packet;
      packet.set_str("hello ...");
      connector->send(&packet);
      last_time = tickcount;
    }
  }
  engine.enqueue([&engine, &mconnector](){ main_nconnect(engine, mconnector); });
}

//DB test.
void db_test(pf_engine::Kernel &engine) {
  auto db = engine.get_db();
  if (is_null(db)) return;
  if (db->isready()) {
    db_query_t db_query;
    pf_db::Query query(&db_query);
    if (!query.init(db)) return;
    query.set_tablename("t_test");
    query.select("*");
    query.from();
    query.limit(1);
    if (query.execute()) {
      pf_basic::io_cwarn("------------------------db---------------------------");
      db_fetch_array_t fectch_array;
      query.fetcharray(fectch_array);
      pf_basic::io_cdebug("db_test keys: ");
      for (pf_basic::type::variable_t &key : fectch_array.keys)
        std::cout << key.string() << std::endl;
      pf_basic::io_cdebug("db_test values: ");
      for (pf_basic::type::variable_t &val : fectch_array.values)
        std::cout << val.string() << std::endl;
      pf_basic::io_cwarn("------------------------db---------------------------");
    }
  } else {
    engine.enqueue([&engine](){ db_test(engine); });
  }
}

int32_t main(int32_t argc, char * argv[]) {
  /* Base config. */
  GLOBALS["app.debug"] = true;
  GLOBALS["app.name"] = "simple";

  //Net.
  GLOBALS["default.net.open"] = true;
  GLOBALS["default.net.service"] = true;
  GLOBALS["default.net.service_port"] = 12306;

  //DB.
  GLOBALS["default.db.open"] = true;
  GLOBALS["default.db.server"] = "pf_test";
  GLOBALS["default.db.user"] = "root";
  GLOBALS["default.db.password"] = "mysql";
  
  //Script.
  GLOBALS["default.script.open"] = true;

  /* engine. */
  pf_engine::Kernel engine;
  pf_engine::Application app(&engine);

  /* command handler. */
  app.register_commandhandler("--reload", "lua script reload.", reload);

  /* engine event. */
  engine.enqueue([](){ std::cout << "main loop function1" << std::endl; });
  engine.enqueue([&engine](){ main_loop(engine); });
  engine.enqueue([&engine](){ main_loop1(engine); });
  engine.enqueue([&engine](){ db_test(engine); });

  /* net init. */
  pf_net::connection::manager::Connector mconnector;
  init_net_packets();
  mconnector.init(1);
  engine.enqueue([&engine, &mconnector](){ main_nconnect(engine, mconnector); });

  /* run */
  app.run(argc, argv);
  return 0;
}
