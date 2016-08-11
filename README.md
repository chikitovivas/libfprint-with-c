# libfprint-with-c
Uso de la libreria libfprint para reconocimiento de huellas con digital Persona 4500

# Instalacion en Linux de Sqlite y libfprint
       
    -sudo apt-get install sqlite3
    -sudo apt-get install libfprint-dev

#BLOB 
    Bloque binario, tipo de dato en la cual se guardara los datos de la huella cuando se transformen a buffer

#Crear BD
    -sqlite3 usuariosH.bd < usuarios.sql

#Registrar un huella
    -gcc registrar.c -o registrar -L../libfprint/.libs -lfprint -lsqlite3
    - ./registrar <num_usuario>

#Identificar una huella
    -gcc identificar.c -o identificar -L../libfprint/.libs -lfprint -lsqlite3
    - ./identificar

#Funciones claves
  Funcion para enrrollar/guardar nueva huella y transformarla en data tipo 'fp_print_data' 
  
          r = fp_enroll_finger_img(dev, &enrolled_print, &img);
                -r : int(error)
                -dev : fp_dev(driver de la huella)
                -enrolled_print: fp_print_data "Tipo de dato que libreria usa para guardar la informacion de la huella"
                -img: fp_img (imagen)
        
  Funcion para transformar la data "fp_print_data" y transformarla en un buffer (unsigned char*) para ser poder ser guardada en BD como BLOB
  
          len = fp_print_data_get_data(data, &buf);
                -len : int (size del buffer, necesario para el guardado y la obtencion del buffer)
                -data : enrolled_print "fp_print_data"
                -buf : buffer(unsigned char*) en donde se guardara la informacion transformada
        
  Funcion para transformar el buffer (ya transformado con fp_print_data_get_data) de unsigned char* para "fp_print_data", para poder ser manejado por la libreria
  
          fdata = fp_print_data_from_data(buf,len)
                -fdata : "fp_print_data" 
                -buf : buffer transformado
                -len : size del buffer

#TIPS
  Identify 
  
        Buscar huella entre muchas huellas (o galeria como ellos lo llaman), para ese tipo de funciones
        que son identify se les tendra que pasar un arreglo del tipo "fp_print_data" (ya enrroladas)
        para que la funcion busque entre todas esas la huella a introducir.
        
  Verify 
  
        Verificar si una huella es igual a otra, para este tipo de verify se le pasa una huella del 
        tipo "fp_print_data"  para ver si hace MATCH con la huella a introducir.
    
