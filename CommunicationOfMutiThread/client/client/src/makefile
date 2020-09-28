cc = g++
target=client
deps=$(shell find ./ -name "*.h")
src=$(shell find ./ -name "*.cpp")
obj=${src:%.cpp=%.o}
${target}:${obj}
	${cc} -o ${target} ${obj} -lpthread
${%.o}:${%.c} ${deps}
	${cc} -c $< -o $@
clean:
	rm -rf ${obj} ${target}
