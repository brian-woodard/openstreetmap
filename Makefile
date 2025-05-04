CXXFLAGS = -g -I. -Iglm -Iimgui -Iimgui/backends

all:
#	g++ $(CXXFLAGS) -c imgui/imgui.cpp -o imgui.o
#	g++ $(CXXFLAGS) -c imgui/imgui_draw.cpp -o imgui_draw.o
#	g++ $(CXXFLAGS) -c imgui/imgui_tables.cpp -o imgui_tables.o
#	g++ $(CXXFLAGS) -c imgui/imgui_widgets.cpp -o imgui_widgets.o
#	g++ $(CXXFLAGS) -c imgui/backends/imgui_impl_glfw.cpp -o imgui_impl_glfw.o
#	g++ $(CXXFLAGS) -c imgui/backends/imgui_impl_opengl3.cpp -o imgui_impl_opengl3.o
#	g++ $(CXXFLAGS) -c -DJSON_IS_AMALGAMATION jsoncpp.cpp -o jsoncpp.o
#	g++ $(CXXFLAGS) -c GlObject.cpp -o GlObject.o
#	g++ $(CXXFLAGS) -c GlLineStrip.cpp -o GlLineStrip.o
#	g++ $(CXXFLAGS) -c GlRect.cpp -o GlRect.o
#	g++ $(CXXFLAGS) -c Shader.cpp -o Shader.o
#	g++ $(CXXFLAGS) -c Texture.cpp -o Texture.o
#	g++ $(CXXFLAGS) -c WmtsIf.cpp -o WmtsIf.o
	g++ $(CXXFLAGS) -c OpenStreetMap.cpp -o OpenStreetMap.o
	g++ $(CXXFLAGS) main.cpp -o main -lglfw GlObject.o GlLineStrip.o GlRect.o Shader.o WmtsIf.o Texture.o OpenStreetMap.o glad/glad.o imgui.o imgui_draw.o imgui_tables.o imgui_widgets.o imgui_impl_glfw.o imgui_impl_opengl3.o exec.a jsoncpp.o -lcurl

clean:
	rm -f main
	rm -f *.o
