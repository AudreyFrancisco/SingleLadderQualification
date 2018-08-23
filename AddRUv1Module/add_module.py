import sys
import os

def main():
   TRUv1str = "TRUv1"
   modstr = sys.argv[1]

   regfile = open(sys.argv[2], "r")
   reglines = regfile.readlines()
   regfile.close()
   total_str = TRUv1str + modstr
   headerfile = open("../RUv1Src/" + total_str + ".h", "w")

   headerfile.write("#ifndef " + total_str.upper() + "_H\n")
   headerfile.write("#define " + total_str.upper() + "_H\n")
   headerfile.write("\n")
   headerfile.write("#include <cstdint>\n#include <iostream>\n")
   headerfile.write("\n")
   headerfile.write("#include \"TRUv1WishboneModule.h\" \n")
   headerfile.write("\n")
   headerfile.write("class " + total_str + " : public TRUv1WishboneModule { \npublic:\n")
   for line in reglines:
       if line.split()[1] == "=":
           headerfile.write("  static const uint16_t " + line.split()[0] + " = " + line.split()[2] + ";\n")
       else:
           headerfile.write("  static const uint16_t " + line.split()[0] + " = " + line.split()[1] + ";\n")
   headerfile.write("\n")
   headerfile.write("   " + total_str + "(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging = false);\n")
   headerfile.write("\n")
   headerfile.write("   void DumpConfig();\n")
   headerfile.write("};\n")
   headerfile.write("\n")
   headerfile.write("#endif // " + total_str.upper() + "_H")
   headerfile.close()

   sourcefile = open("../RUv1Src/" + total_str + ".cpp", "w")
   sourcefile.write("#include \"" + total_str + ".h\"\n")
   sourcefile.write("\n")
   sourcefile.write("#include <iostream> \n")
   sourcefile.write("\n")
   sourcefile.write("#include \"TReadoutBoardRUv1.h\"\n")
   sourcefile.write("\n")
   sourcefile.write(total_str + "::" + total_str + "(TReadoutBoardRUv1 &board, uint8_t moduleId, bool logging)\n     : TRUv1WishboneModule(board, moduleId, logging)\n{\n}\n")
   sourcefile.write("\n")
   sourcefile.write("void " + total_str + "::DumpConfig()\n")
   sourcefile.write("{\n")
   sourcefile.write("   std::cout << \"...." + total_str.upper() + " MODULE CONFIG....\\n\"; \n")
   sourcefile.write("   for(int i = 0; i < " + str(len(reglines)) + "; i++){\n")
   sourcefile.write("       std::cout << \"ADDRESS \" << i << \" HAS VALUE \" << Read(i, true) << \"\\n\";\n")
   sourcefile.write("   }\n")
   sourcefile.write("}\n")
   sourcefile.close()

   if len(sys.argv) > 3:
       if sys.argv[3] == "make":
           os.chdir("..")
           os.system("make format")
           os.chdir("AddRUv1Module")


if __name__ == "__main__":
   main()
