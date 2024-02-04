add_executable(violet_robot
	txt_to_arduino.c
	)

target_link_libraries(violet_robot pico_stdlib pico_multicore)

pico_add_extra_outputs(violet_robot)
example_auto_set_url(violet_robot)
