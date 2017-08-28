#!/bin/bash

#############################################################
# Use this script to generate firmware update packages
# it will generate it at ~/firmware-update-<git commit>.tgz
#
# 2015-03-12 ab: We now make clean && make with the variables
#					set to whats needed for a release build.
#############################################################

GIT_VERSION=$(git log --pretty=format:%h -1)

#at some point, we might want to check if the display files changed and include them only if so.
# for now, we always copy them
#COPY_DISPLAY_IMGS=$(git whatchanged -n 1 | grep DisplayE.dat)
COPY_DISPLAY_IMGS=Yes

FOLDER_NAME="firmware-$GIT_VERSION"
DIR_NAME="$HOME/$FOLDER_NAME"
SPWD=$(pwd)

make clean && make -j SPLIT_CONFIG_AND_PUBLIC_SD=1 BUILD_TYPE=Checked ENABLE_DEBUG_MONITOR=0

if [ ! -d $DIR_NAME ]; then
    echo  "Creating dir $DIR_NAME"
    mkdir "$DIR_NAME"
else
    echo  "Deleting files from $DIR_NAME"
    rm "$DIR_NAME/"*
fi

if [ ! -d "$DIR_NAME/display/" ]; then
    echo  "Creating dir $DIR_NAME"
    mkdir "$DIR_NAME/display"
else
    echo  "Deleting files from $DIR_NAME"
    rm "$DIR_NAME/display/"*
fi

if [ ! -d "$DIR_NAME/config-factories/" ]; then
    echo  "Creating dir $DIR_NAME"
    mkdir "$DIR_NAME/config-factories"
else
    echo  "Deleting files from $DIR_NAME"
    rm "$DIR_NAME/config-factories/"*
fi

if [ "$COPY_DISPLAY_IMGS" != "" ]; then
    echo "Copying display files..."
    cp display_sd/* "$DIR_NAME/display/"
fi

echo "Copying smoothie firmware file..."
cp LPC1768/main.bin "$DIR_NAME/FIRMWARE.BIN"
cp config_sd/config "$DIR_NAME/config"
cp config_sd/config-priv "$DIR_NAME/config-priv"
cp config_sd/config-factories/* "$DIR_NAME/config-factories"
cp config_sd/on_boot.gcode "$DIR_NAME/on_boot.gcode"
echo "Compressing debugging information..."
tar cfJ LPC1768.txz --exclude=main.disasm --exclude=main.hex --exclude=main.bin LPC1768/
mv LPC1768.txz "$DIR_NAME"

echo "Creating instructions file..."
cat > "$DIR_NAME/instrucciones.txt" << EOF
Para realizar el update:
- Copiar los archivos FIRMWARE.BIN y KLCONFIG.CNF a una tarjeta SD.
- Copiar la carpeta display a la tarjeta SD donde recien se copiaron los archivos (incluir la carpeta).
- Insertar la tarjeta SD en el panel frontal de la impresora y reiniciarla.
- La impresora deberia comenzar sola el proceso de update, y al cabo de unos minutos reiniciarse sola.

Para cargar firmware por primera vez:
- Copiar los archivos FIRMWARE.BIN, config, config-priv y on_boot a la tarjeta SD que va dentro de la placa smoothie.
- De los archivos incluidos en la carpeta config-factories, elegir el mas adecuado de acuerdo al tipo de impresora y copiarlo junto con los archivos recien copiados, cambiandole el nombre a config-factory. No copiar la carpeta, solo el archivo.
- Copiar el contenido de la tarjeta display (sin la carpeta en si) a la tarjeta SD que va dentro del display LCD.
- Prender la impresora. Los archivos deberian ser cargados inmediatamente.
EOF



echo "Creating .tgz ..."
cd "$DIR_NAME" && tar -zcvf "../$FOLDER_NAME.tgz" .

echo "Deleting created folder..."
rm -r "$DIR_NAME"


# Go back to project folder and start again with the zip
cd "$SPWD"

FOLDER_NAME="firmware-update-$GIT_VERSION"
DIR_NAME="$HOME/$FOLDER_NAME"

echo "Creating update zip file..."
if [ ! -d $DIR_NAME ]; then
    echo  "Creating dir $DIR_NAME"
    mkdir "$DIR_NAME"
else
    echo  "Deleting files from $DIR_NAME"
    rm "$DIR_NAME/"*
fi

if [ ! -d "$DIR_NAME/display/" ]; then
    echo  "Creating dir $DIR_NAME"
    mkdir "$DIR_NAME/display"
else
    echo  "Deleting files from $DIR_NAME"
    rm "$DIR_NAME/display/"*
fi

cp LPC1768/main.bin "$DIR_NAME/FIRMWARE.BIN"
cp config_sd/config "$DIR_NAME/KLCONFIG.CNF"

if [ "$COPY_DISPLAY_IMGS" != "" ]; then
    echo "Copying display files..."
    cp display_sd/* "$DIR_NAME/display/"
fi

echo "Creating release notes file..."
cat > "$DIR_NAME/notas.txt" << EOF
Instalación
***********
El firmware es el software que se ejecuta en la placa procesadora de la impresora, la Smoothie. Contiene el programa que permite a la impresora entender el gcode para accionar los mecanismos como motores, luces, pantalla, sensores, etc. Periodicamente Kikai Labs hace mejoras a este software, para corregir defectos o agregar nueva o mejorada funcionalidad.  En la pantalla de información puede ver qué versión de firmware tiene instalada. Si es distinta de la publicada acá, por favor actualicela de la siguiente manera:
- Descargue el archivo con la última versión desde esta página.
- Extraiga el contenido del .ZIP (FIRMWARE.BIN, KLCONFIG.CNF y la carpeta "display")  y cópielos a la tarjeta SD, sin cambiarle el nombre a los archivos. La tarjeta SD está accesible normalmente desde el Finder (Mac) o el Explorer (Windows), por lo tanto puede arrastrar el archivo para copiarlo.
- Eyecte la tarjeta normalmente de su PC/Mac
- Inserte la tarjeta en la ranura a la derecha de la pantalla color con la impresora apagada. Encienda la impresora.
- Al encontrar el archivo del firmware, la impresora lo instalará, un proceso que llevará unos pocos minutos. NO INTERRUMPA la operación, no apague la impresora, por favor sea paciente hasta que la pantalla indique que el proceso ha terminado. La pantalla se reiniciará al finalizar correctamente y el equipo estará actualizado. Si por algún motivo no termina (parece trabarse la actualización), pruebe de apagar y reencender el equipo).

EOF
echo "Notas de la Version $GIT_VERSION:" >> "$DIR_NAME/notas.txt"
cat release-notes.txt >> "$DIR_NAME/notas.txt"

echo "Creating .zip ..."
cd "$DIR_NAME" && zip -r "../$FOLDER_NAME.zip" .

echo "Deleting created folder..."
rm -r "$DIR_NAME"

echo "Done!"
