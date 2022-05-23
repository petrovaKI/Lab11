// Copyright 2022 Petrova Kseniya <petrovaKI>
#include "my_cmake.hpp"
namespace chrono = std::chrono;

void start_cmake(int argc, char* argv[]) {
  po::options_description desc("Options");
  desc.add_options()
      ("help", "help information")
          ("config", po::value<std::string>(),
              "конфигурация сборки (по умолчанию Debug)")
              ("install",
               "этап установки (в директорию _install)")
                  ("pack",
                   "этап упаковки (в архив формата tar.gz)")
                      ("timeout", po::value<time_t>(),
                          "время ожидания (в секундах)");

  po::variables_map vm;
  po::store(parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  //если аргументы не заданы выводим информационное сообщение
  if (vm.count("help") && !vm.count("config")  && !vm.count("pack")
      && !vm.count("timeout") && !vm.count("install")) {
    std::cout << desc << "\n";
  }else{
    //конфигурация по умолчанию debug
    std::string config = "Debug";
    //фиксируем текущее время
    time_t timeout =  time_now();
    time_t time_spent = 0;
    if (vm.count("timeout")) {
      timeout = vm["timeout"].as<time_t>();
    }

    if (vm.count("config")) {
      config = vm["config"].as<std::string>();
    }

    std::string cmake_1 = "cmake -H. -B_builds -DCMAKE_INSTALL_" +
                          std::string("PREFIX=_install -DCMAKE_BUILD_TYPE=");
    std::string cmake_2 = "cmake --build _builds";
    std::string cmake_3 = "cmake --build _builds --target install";
    std::string cmake_4 = "cmake --build _builds --target package";

    if (config == "Debug" || config == "Release") {
      int build_flag = 0;
      int install_flag = 0;
      //задаём конфигурацию сборки
      cmake_1 += config;
      //Наиболее распространенным способом создания задачи является
      // использование spawn()функции, которая будет асинхронно
      // запускать данную функцию в пуле потоков и возвращать задачу,
      // связанную с результатом функции
      auto build_ = async::spawn([&build_flag, config, timeout,
                              &time_spent, cmake_1, cmake_2] () mutable {
        //фиксируем текущее время
        time_t start_config = time_now();
        //запускаем процесс по установке конфигурации сборки
        create_child(cmake_1, timeout);
        //фиксируем время
        time_t end_config = time_now();
        //фиксируем длительность выполнения
        time_spent += end_config - start_config;
        //максимальное время для выполнения сборки
        time_t max_build_duration = timeout - time_spent;

        //запускаем сборку
        create_child(cmake_2, max_build_duration, build_flag);

        time_t end_build = time_now();
        //фиксируем длительность сборки
        time_spent += end_build - end_config;
      });
      //если сборку успешна - переходим к установке
      if (vm.count("install") && build_flag == 0) {
        auto install_ = build_.then
                        ([&install_flag, cmake_3,
                           timeout, &time_spent] () mutable {
          //максимальное время для выполнения  установки
          time_t max_install_duration = timeout - time_spent;

          time_t start_install = time_now();

          //запускаем установку
          create_child(cmake_3, max_install_duration,
                       install_flag);

          time_t end_install = time_now();

          time_spent += end_install - start_install;
        });
        //в случае успешной установки выполняем упаковку
        if (vm.count("pack") && install_flag == 0) {
          auto pack_ = install_.then
                       ([cmake_4, timeout, time_spent] () {
            time_t max_pack_duration = timeout - time_spent;

            create_child(cmake_4, max_pack_duration);
          });
        }
      }
    }else{
      std::cerr << "config = " << config << " doesn't exist!\n";
    }
  }
}

void create_child(const std::string& command, const time_t& period) {
  std::string line;
  processes::ipstream out;

  processes::child process(command, processes::std_out > out);

  std::thread checkTime(check_time, std::ref(process),
                        std::ref(period));

  while (out && std::getline(out, line) && !line.empty())
    std::cerr << line << std::endl;

  checkTime.join();
}

void create_child(const std::string& command, const time_t& period, int& res) {
  std::string line;
  processes::ipstream out;
//std_out - выходной поток для дочернего процесса
  processes::child process(command, processes::std_out > out);

  std::thread checkTime(check_time, std::ref(process),
                        std::ref(period));

  while (out && std::getline(out, line) && !line.empty())
    std::cerr << line << std::endl;

  checkTime.join();

  res = process.exit_code();
}

void check_time(processes::child& process, const time_t& period) {
  time_t start = time_now();

  while (true) {
    if ((time_now() - start > period) && process.running()) {
      std::error_code ec;
      process.terminate(ec);
      std::cout << ec;
      break;
    }
    else if (!process.running()) {
      break;
    }
  }
}

time_t time_now() {
  return chrono::system_clock::to_time_t(
      chrono::system_clock::now());
}
