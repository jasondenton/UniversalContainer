#include <cstdio>
#include "../univcont.h"
#include "../uccontract.h"
using namespace std;
using namespace JAD;


int main(int argc, char** argv)
{
  
  try {
    printf("Data to be verified -------\n");
    //load the data to be verified.
    UniversalContainer data = uc_from_json_file("example_schema.json");
    print_json(data);
    Buffer* schema = read_to_buffer("example_schema.contract");
    schema->put('\0'); //add a null terminator so we can write_from_buffer to stdout.
    printf("\nSchema to be checked ------\n");
    write_from_buffer(schema,stdout);
    printf("\n------------\n");
    schema->rewind();
    print_json(data["vehicle"]);
    printf("\n------------\n");
    UCSchema::add_contract_to_library(schema);
    printf("Added the contract\n");
    UCSchema::compare_and_throw("car", data["vehicle"]);
    printf("Contract matched\n");

  }
  catch(UniversalContainer uce) {
    printf("An exception was thrown.\n");
    print_json(uce);
  }
}
