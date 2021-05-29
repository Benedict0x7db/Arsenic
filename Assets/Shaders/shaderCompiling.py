import os

from distutils.dir_util import copy_tree

def compileShader():
    #os.system('cmd /c "glslc vertex.vert -o Spv/vertex.vert.spv"')
    #os.system('cmd /c "glslc pbr.frag -o Spv/pbr.frag.spv"')
    #os.system('cmd /c "glslc skybox.vert -o Spv/skybox.vert.spv')
    #os.system('cmd /c "glslc skybox.frag -o Spv/skybox.frag.spv')

    os.system('cmd /c "glslc rtCompute.comp -o Spv/rtCompute.comp.spv"')
    os.system('cmd /c "glslc fullScreen.vert -o Spv/fullScreen.vert.spv"')
    os.system('cmd /c "glslc fullScreen.frag -o Spv/fullScreen.frag.spv"')
compileShader()