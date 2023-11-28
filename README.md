## Requerimientos
En primera instancia, se debe instalar el entorno de docker, corriendo el siguiente comando en la terminal.
```bash
docker pull agodio/itba-so:1.0
```  
Luego, se requiere iniciar un contenedor de docker con esa imagen con el siguiente comando.
```bash
docker run -v "${PWD}:/root" --privileged -ti agodio/itba-so:1.0
```
Finalmente, accedemos al directorio root del docker para usar el programa.
```bash
cd root
```
## Instrucciones de compilación
Para comenzar con la compilación, este proyecto cuenta con un Makefile que facilita este proceso. Este proyecto cuenta con dos implementaciones de memory manager, "fixed size allocation"
y "buddy", a continuación se detallan los pasos a seguir para compilar usando el primero de ellos.
```bash
cd Toolchain
make clean
make all
cd ..
make clean
make all
```

Si se quisiera usar el buddy, se deben ingresar los siguientes comandos en la terminal.
```bash
cd Toolchain
make clean
make all
cd ..
make clean
make buddy
```

Cualquiera sea el memory manager elegido, luego salimos del docker.
```bash
exit
```

## Instrucciones de ejecución
### Correr el sistema operativo McWhigginOS.
Finalmente, ejecutamos la siguiente línea para correr el programa.
```bash
./run.sh
```
