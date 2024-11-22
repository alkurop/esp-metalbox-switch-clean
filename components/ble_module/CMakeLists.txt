idf_component_register(SRCS 
"esp_hid_gap.c" 
"ble_module.cpp" 
                       
INCLUDE_DIRS "include"
                       
REQUIRES bt nvs_flash esp_hid log timer)


