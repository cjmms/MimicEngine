# **MimicEngine** #
### **"Not a mimic!"** ###

### **Features** ###
* importing materials that support PBR:
  * Diffuse, normal, specular, AO, normal mapping. Textures are imported as bindless GL textures through stb_image.h
  * 3D Model file with .obj format through my code and Assimp library
* PBR
* Deferred Rendering Pipeline
* Basic Shadow Mapping (Might replaced by CSM later if time available)
* Camera
* Basic HDR and Gamma Correction

# **Controls:** #

### **Keyboard:** ###

* WASD = Movement

### **Mouse:** ###

* Moving mouse = Changing camera direction (just like FPS game)



# 3rd Party Libraries:

Loading image
https://github.com/nothings/stb

Loading model
http://assimp.org/

Math
https://github.com/g-truc/glm

GLEW
http://glew.sourceforge.net/

GLFW
https://www.glfw.org/
