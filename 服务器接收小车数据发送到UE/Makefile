# 定义编译器
CXX = g++

# 定义编译器标志
CXXFLAGS = -Wall -std=c++11

# 定义目标文件
TARGET = SendDate_Car_TO_UE

# 定义源文件
SRCS = main.cpp server.cpp

# 定义头文件
HEADERS = server.h

# 定义目标文件的生成规则
$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# 定义清理规则
.PHONY: clean
clean:
	rm -f $(TARGET)
