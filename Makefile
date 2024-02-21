
project_dir=$(shell pwd)
sdk_dir=/home/nihao/work/zephyr/work/zephyrproject/zephyr

myboard=esp32


target : 
	bash -c "source ${sdk_dir}/zephyr-env.sh ; west build -p auto -b ${myboard} ${project_dir} "

menuconfig:
	bash -c "source ${sdk_dir}/zephyr-env.sh ; west build -p auto -b ${myboard} ${project_dir} -t menuconfig"

flash:
	bash -c "source ${sdk_dir}/zephyr-env.sh ; west flash --esp-baud-rate 1500000"

clean:
	rm build -r