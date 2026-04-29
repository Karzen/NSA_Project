# cmake/BinToHeader.cmake
file(READ ${INPUT_FILE} HEX_CONTENTS HEX)

string(REGEX MATCHALL "([0-9a-f][0-9a-f])" HEX_LIST "${HEX_CONTENTS}")
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1" C_ARRAY_TMP "${HEX_LIST}")
string(REPLACE ";" ", " C_ARRAY "${C_ARRAY_TMP}")

list(LENGTH HEX_LIST HEX_LIST_COUNT)

file(WRITE ${OUTPUT_FILE} "#pragma once\n")
file(APPEND ${OUTPUT_FILE} "inline const unsigned char raw_dll[] = { ${C_ARRAY} };\n")
file(APPEND ${OUTPUT_FILE} "inline unsigned int raw_dll_len = ${HEX_LIST_COUNT};\n")