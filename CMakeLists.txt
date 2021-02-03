cmake_minimum_required(VERSION 2.8)	 # Проверка версии CMake.
										# Если версия установленой программы
										# старее указаной, произайдёт аварийный выход.

set(SOURCE main.cpp)		# Установка переменной со списком исходников для исполняемого файла

set(SOURCE_LIB1 Settings.cpp)		# Тоже самое, но для библиотеки
set(SOURCE_LIB2 Photon.cpp)
set(SOURCE_LIB3 Event.cpp)
set(SOURCE_LIB4 App.cpp)

add_library(Settings STATIC ${SOURCE_LIB1})	# Создание статической библиотеки
add_library(Photon STATIC ${SOURCE_LIB2})
add_library(Event STATIC ${SOURCE_LIB3})
add_library(App STATIC ${SOURCE_LIB4})

add_executable(get_offline_reduced ${SOURCE})	# Создает исполняемый файл

target_link_libraries(get_offline_reduced App Event Photon Settings)		# Линковка программы с библиотекой
