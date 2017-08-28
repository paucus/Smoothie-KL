/*
 * LCDTranslations.cpp
 *
 *  Created on: Feb 23, 2015
 *      Author: abialet
 */

#include "LCDTranslations.h"
#include "string.h"
#include "RobotConsts.h"
#include <ctype.h>
#include <stdlib.h>

static const char *translations[3][NUMBER_OF_ITEMS] =
		{
				{
						"Language",
						"Serial number: %s",
						"Firmware: %s",
						"No files available.",
						"Autoleveling",
						"Please wait.",
						"Current temperature",
						"Change Filament",
						"Retract until you can remove current filament, push\nnew filament until it reaches the hotend and extrude\nuntil it comes out. Use pliers to clean the hotend.",
						"Printing",
						"Elapsed time:",
						"Remaining time:",
						"Percentage complete:",
						"Temperatures:",
						"Hotend temperature:",
						"Bed temperature:",
						"Speed modifier:",
						"Filament flow rate:",
						"Printing done",
						"Please, wait until the bed cools down before touching it.",
						"You can now remove the print from the platform.",
						"Cancel Print",
						"Are you sure you want to cancel this print?",
						"Print paused",
						"You can resume printing or switch filament before continuing.",
						"The file '%s' can not be printed. Please, check that the sd card is inserted and the file was not deleted.",
						"Cold extrusion prevented. Heat your extruder and try again.",
						"Filament",
						"Home",
						"Platform",
						"Copying file %d of %d",
						"Updating failed, please try again.",
						"Display update successful.",
						"Error while initializing the printer",
						"Please, verify that the configuration card is correctly inserted in the controller board, and then restart the printer",
						"Error while initializing the printer",
						"Please, verify that the volume is correctly configured, and then restart the printer. Maximum XYZ allowed: " X_UPPER_BOUND_STR "," Y_UPPER_BOUND_STR "," Z_UPPER_BOUND_STR,
						"Heating, please wait...",
						"Extruder",
						"Please, choose the heater to tune",
						"Running PID autotuning",
						"Run PID test?",
						"Running PID test",
						"PID test result",
						"Peak:",
						"Target:",
						"Time (Stab):",
						"Time (Reach):",
						"Save PID values?",
						"Calibration step %d of %d. Target temperatures: %d/%d�C",
						"Network",
						"Settings",
						"Analyzing file...",
						"Display Settings",
						"Turn off when unused %s",
						"Calibration",
						"Z Probe Calibration",
						"For each temperature adjust the nozzle height with the arrows until it touches the surface. Press next to begin.",
						"PID Tuning",
						"Slicer: ",
						"Layer Height: ",
						"Print time: ",
						"Extruder temp: ",
						"Bed temp: ",
						"Filament usage: ",
						"File size: ",
						"File date: ",
						"Restart printer",
						"Restart printer",
						"Are you sure you want to restart the printer?",
						"Upgrade firmware",
						"Upgrade firmware",
						"Are you sure you want to search for an upgrade of the firmware in the SD card?",
						"MINTEMP or MAXTEMP triggered. Check your temperature sensors are correctly connected and working.",
						"MINTEMP or MAXTEMP triggered for the hotend. Check your temperature sensors are correctly connected and working.",
						"MINTEMP or MAXTEMP triggered for the heatbed. Check your temperature sensors are correctly connected and working.",
						"A significant temperature drop was detected. Check the hotend thermistor is correctly attached to the heater block.",
				},
				{
						"Idioma",
						"N�mero de serie: %s",
						"Versi�n de firmware: %s",
						"No hay archivos disponibles",
						"Nivelando la plataforma",
						"Por favor espere hasta que el proceso finalice.",
						"Temperatura actual",
						"Cambio de filamento",
						"Retraiga el filamento actual hasta que pueda retirarlo,\nempuje el nuevo hasta hacer tope y extruya hasta que\nsalga por el pico. Utilice las pinzas para limpiar.",
						"Imprimiendo",
						"Tiempo transcurrido:",
						"Tiempo restante:",
						"Completado:",
						"Temperaturas:",
						"Temperatura de extrusor:",
						"Temperatura de cama:",
						"Velocidad:",
						"Flujo de filamento:",
						"Impresi�n finalizada",
						"Por favor, aguarde hasta que la cama se enfr�e antes de tocarla.",
						"Ahora puede quitar la impresi�n de la plataforma.",
						"Cancelar impresi�n",
						"�Est� seguro que desea cancelar esta impresi�n?",
						"Impresi�n pausada",
						"Puede continuar la impresi�n o cambiar el filamento antes de continuar.",
						"El archivo '%s' no est� disponible. Por favor verifique que la tarjeta SD est� insertada, y el archivo no fu� borrado.",
						"El extrusor no est� a la temperatura m�nima requerida para extruir. Caliente el extrusor e intente nuevamente.",
						"Filamento",
						"Inicio",
						"Plataforma",
						/* These texts are shown when custom fonts are unavailable, so they NEED to use
						 * ASCII characters only. */
						"Copiando archivo %d de %d",
						"La actualizaci�n fallo, intente nuevamente.",
						"Sistema actualizado.",
						"Error al iniciar la impresora",
						"Por favor, verifique que la tarjeta de configuraci�n se encuentre correctamente colocada en la placa controladora, y luego reinicie la impresora",
						"Error al iniciar la impresora",
						"Por favor, verifique que el volumen est� correctamente configurado, y luego reinicie la impresora. M�ximo XYZ permitido: " X_UPPER_BOUND_STR "," Y_UPPER_BOUND_STR "," Z_UPPER_BOUND_STR ".",
				/* End of ASCII only characters */
						"Calentando, por favor espere...",
						"Extrusor",
						"Elija el calentador al cual estimar el PID",
						"Corriendo estimaci�n de PID",
						"�Correr test de PID?",
						"Corriendo test de PID",
						"Resultado de test",
						"Pico:",
						"Temperatura:",
						"Tiempo (Estab):",
						"Tiempo (Alcanzar):",
						"�Guardar valores de PID?",
						"Paso de calibraci�n %d de %d. Temperaturas: %d/%d�C",
						"Red",
						"Configuraci�n",
						"Analizando archivo...",
						"Configuraci�n de pantalla",
						"Apagar cuando no se utilice: %s",
						"Calibraci�n",
						"Calibraci�n de Sensor Z",
						"Para cada temperatura ajuste la altura de la punta del extrusor utilizando las flechas hasta que toque la superficie. Presione siguiente para comenzar.",
						"Ajuste de PID",
						"Slicer: ",
						"Layer Height: ",
						"Tiempo de impresi�n: ",
						"Temp de extrusor: ",
						"Temp de cama: ",
						"Uso de filamento: ",
						"Tama�o de archivo: ",
						"Fecha de archivo: ",
						"Reiniciar impresora",
						"Reiniciar impresora",
						"Est� seguro que desea reiniciar la impresora?",
						"Actualizar firmware",
						"Actualizar firmware",
						"Est� seguro que desea que se busque una actualizaci�n de firmware en la tarjeta SD?",
						"Se dispar� un evento de MINTEMP o MAXTEMP. Por favor, verifique que los sensores de temperatura est�n conectados y funcionando correctamente.",
						"Se dispar� un evento de MINTEMP o MAXTEMP en el hotend. Por favor, verifique que los sensores de temperatura est�n conectados y funcionando correctamente.",
						"Se dispar� un evento de MINTEMP o MAXTEMP en la cama calefaccionada. Por favor, verifique que los sensores de temperatura est�n conectados y funcionando correctamente.",
						"Un descenso significativo de temperatura fue detectado. Por favor, verifique que el termistor del hotend est� correctamente conectado al heater block.",
				},
				{
						"Linguagem",
						"N�mero de s�rie: %s",
						"Vers�o do firmware: %s",
						"Nenhum arquivo dispon�vel",
						"Nivelamento autom�tico",
						"Por favor, aguarde at� que o processo seja conclu�do",
						"Temperatura atual",
						"Trocar filamento",
						"Recolha o segmento atual at� que voc� pode remover, empurrar para tr�s at� que ele pare e expulsar at� que ele apare�a no pico . Use uma pin�a para limpar.",
						"Imprimindo",
						"Tempo decorrido:",
						"Tempo restante:",
						"Conclu�do:",
						"Temperaturas:",
						"Temperatura de extrus�o:",
						"Temperatura da superfice:",
						"Velocidad:",
						"Fluxo de filamentos:",
						"Impress�o conclu�da",
						"Por favor, aguarde at� que a cama esfriar antes de toc�-lo.",
						"Agora voc� pode remover a impress�o a partir da plataforma.",
						"Cancelar impress�o",
						"Tem certeza que deseja cancelar essa impress�o?",
						"Impress�o pausada",
						"Voc� pode retomar a impress�o ou mudar filamento antes de continuar.",
						"O arquivo '%s' n�o pode ser impresso. Por favor, verifique se o cart�o SD est� inserido e que o arquivo n�o foi exclu�do.",
						"Extrus�o a frio impedido. Aque�a o extrusora e tente novamente.",
						"Expulsar",
						"Inicio",
						"Plataforma",
						/* These texts are shown when custom fonts are unavailable, so they NEED to use
						 * ASCII characters only. */
						"Copiando o arquivo %d de %d",
						"Falha na atualiza��o, por favor tente novamente.",
						"Atualiza��o bem-sucedida. O sistema ir� reiniciar agora.",
						"Falha ao iniciar a impressora",
						"Por favor, verifique se o cart�o de configura��o est� devidamente colocado na placa do controlador, e em seguida, reiniciar a impressora",
						"Falha ao iniciar a impressora",
						"Por favor, verifique se o volume est� definido corretamente, e em seguida, reiniciar a impressora. M�ximo XYZ permitido: " X_UPPER_BOUND_STR "," Y_UPPER_BOUND_STR "," Z_UPPER_BOUND_STR,
				/* End of ASCII only characters */
						"Aquecimento, por favor aguarde ...",
						"Extrus�o",
						"Escolha o aquecedor para afinar",
						"Executando estimativa do PID",
						"Proba de PID",
						"Executando proba de PID",
						"Resultado de proba",
						"Pico:",
						"Temperatura:",
						"Tempo (Estab):",
						"Tempo (Alcan�ar):",
						"�Salvar valores de PID?",
						"Passo de Calibra��o %d de %d. Temperaturas: %d/%d�C",
						"Rede",
						"Configura��o",
						"Analisando arquivo...",
						"Configura��o de exibi��o",
						"Desligue quando n�o for utilizado: %s",
						"Calibra��o",
						"Calibra��o da sonda Z",
						"Para cada temperatura fixar a altura da ponta da extrusora utilizando a seta at� que ela toque a superf�cie. Clique em Avan�ar para come�ar.",
						"Afina��o de PID",
						"Slicer: ",
						"Layer Height: ",
						"Tempo de impress�o: ",
						"Temp de extrus�o: ",
						"Temp da superficie: ",
						"Uso de filamento: ",
						"Tamanho do arquivo",
						"Data de arquivo: ",
						"Reinicializa��o do impressora",
						"Reinicializa��o do impressora",
						"Tem certeza de que deseja reiniciar a impressora?",
						"Atualiza��o do firmware",
						"Atualiza��o do firmware",
						"Tem certeza de que deseja procurar uma atualiza��o do firmware no cart�o SD?",
						"Um evento de MINTEMP ou MAXTEMP foi disparado. Verifique se os sensores de temperatura est�o conectados corretamente e funcionando.",
						"Um evento de MINTEMP ou MAXTEMP foi disparado pelo hotend. Verifique se os sensores de temperatura est�o conectados corretamente e funcionando.",
						"Um evento de MINTEMP ou MAXTEMP foi disparado pelo cama. Verifique se os sensores de temperatura est�o conectados corretamente e funcionando.",
						"Uma queda significativa da temperatura foi detectado. Verifique se o termistor est� diretamente ligado ao bloco de aquecimento.",
				} };

const char* LCDTranslations::language_t_to_str(language_t t) {
	return (t == ENGLISH) ? "english" : ((t == SPANISH) ? "spanish" : "portuguese");
}

language_t LCDTranslations::str_to_language_t(const char* language) {
	char* l = strdup(language);
	int len = strlen(language);

	for (int i = 0; i < len; i++) {
		l[i] = tolower(language[i]);
	}
	l[len] = '\0';

	language_t ret =
			(strncmp("english", l, 7) == 0) ? ENGLISH : ((strncmp("portuguese", l, 10) == 0) ? PORTUGUESE : SPANISH);

	free(l);
	return ret;
}

const char* LCDTranslations::get_translation(language_t language, message_label_t label) {
	return translations[language][label];
}
