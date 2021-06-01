import os
import shutil

def compileShader():
    print("Before compiling shaders:")

    os.system('cmd /c "glslc Shaders/rtCompute.comp -o Shaders/Spv/rtCompute.comp.spv"')
    os.system('cmd /c "glslc Shaders/fullScreen.vert -o Shaders/Spv/fullScreen.vert.spv"')
    os.system('cmd /c "glslc Shaders/fullScreen.frag -o Shaders/Spv/fullScreen.frag.spv"')

    print("After compiling shaders:")

def copyAssets():
    print("Before copying Assets:")

    src = '../Assets'
    dst = '../build/ArsenicSandbox/Assets'
    dstExist = os.path.exists(dst)
    
    if dstExist:
        shutil.rmtree(dst)
    
    shutil.copytree(src, dst)

    print("After copying Assets:")

compileShader()
copyAssets()