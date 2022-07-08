#include <iostream>
#include <fstream>
#include <cstdlib>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>

int main (int argc, char* argv[])
{   
    std::ifstream ifs { R"(test.json)" };
    if ( !ifs.is_open() )
    {
        std::cerr << "Could not open file for reading\n";
        return EXIT_FAILURE;
    }

    //Перевод std::ifstream ifs на поток rapidjson isw
    rapidjson::IStreamWrapper isw { ifs };

    //Парсим наш json файл из входного потока 
    rapidjson::Document test_document {};
    test_document.ParseStream( isw );

    //Выделяем буфер для записи нашего файла и генерируем текст  
    rapidjson::StringBuffer buffer {};
    rapidjson::Writer<rapidjson::StringBuffer> writer { buffer };

    //Передаем поле "cargo_space"
    test_document[ "cargo_space" ].Accept( writer );

    if ( test_document.HasParseError() )
    {
        std::cout << "Error  : " <<  test_document.GetParseError()  << '\n'
                  << "Offset : " <<  test_document.GetErrorOffset() << '\n';
        return EXIT_FAILURE;
    }

    //Извлекаем строку
    const std::string jsonStr { buffer.GetString() };

    std::cout << jsonStr << '\n';

    system("pause");
    return 0;
}