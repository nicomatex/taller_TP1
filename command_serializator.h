#include "command_parser.h"
#include <stdint.h>

unsigned char* generate_header(command_t* command, uint32_t serial_number, size_t* header_size);

unsigned char* generate_body(command_t* command,size_t* body_size);