// Declaración de la estructura de datos // de una huella captura por el dispositivo

//struct fp_print_data *fdata;

// Coloca en un buffer la información capturada // por el dispositivo y devuelve la longitud

//len = fp_print_data_get_data(data, &buf);

// Convierte los datos almacenados en un // buffer en la estructura de datos fdata

//fdata = fp_print_data_from_data(buf,len);

// Esta es la más importante // toma una huella y realiza la comparación // entre una colección de huellas, almacenada // en fdata que, en este caso, debe ser // un array de punteros

//fp_identify_finger(dev, fdata, &posicion);


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <libfprint/fprint.h>

/* Descrubrir un dispositivo */
struct fp_dscv_dev *discover_device(struct fp_dscv_dev **discovered_devs)
{
	/* dispositivo */
	struct fp_dscv_dev *ddev = discovered_devs[0];
	/* driver */
	struct fp_driver *drv;
	if (!ddev)
		return NULL;
		/* obtener driver */
	drv = fp_dscv_dev_get_driver(ddev);
	printf("Found device claimed by %s driver\n", fp_driver_get_full_name(drv));
	return ddev;
}
/* Enrrolar huella, o guardar una nueva huella */
struct fp_print_data *enroll(struct fp_dev *dev) {
	struct fp_print_data *enrolled_print = NULL;
	int r;

	printf("You will need to successfully scan your finger %d times to "
		"complete the process.\n", fp_dev_get_nr_enroll_stages(dev));
/* Se hara hasta que el enrrolado sea completado, 5 veces exitosas */
	do {
		struct fp_img *img = NULL;

		sleep(1);
		printf("\nScan your finger now.\n");
		/* Para enrolar una vez */
		r = fp_enroll_finger_img(dev, &enrolled_print, &img);
		if (img) {
			fp_img_save_to_file(img, "enrolled.pgm");
			printf("Wrote scanned image to enrolled.pgm\n");
			fp_img_free(img);
		}
		if (r < 0) {
			printf("Enroll failed with error %d\n", r);
			return NULL;
		}

		switch (r) {
		case FP_ENROLL_COMPLETE:
			printf("Enroll complete!\n");
			break;
		case FP_ENROLL_FAIL:
			printf("Enroll failed, something wen't wrong :(\n");
			return NULL;
		case FP_ENROLL_PASS:
			printf("Enroll stage passed. Yay!\n");
			break;
		case FP_ENROLL_RETRY:
			printf("Didn't quite catch that. Please try again.\n");
			break;
		case FP_ENROLL_RETRY_TOO_SHORT:
			printf("Your swipe was too short, please try again.\n");
			break;
		case FP_ENROLL_RETRY_CENTER_FINGER:
			printf("Didn't catch that, please center your finger on the "
				"sensor and try again.\n");
			break;
		case FP_ENROLL_RETRY_REMOVE_FINGER:
			printf("Scan failed, please remove your finger and then try "
				"again.\n");
			break;
		}
	} while (r != FP_ENROLL_COMPLETE); // Saldra cuando se completen las 5 enrolamientos exitosos

	if (!enrolled_print) {
		fprintf(stderr, "Enroll complete but no print?\n");
		return NULL;
	}

	printf("Enrollment completed!\n\n");
	return enrolled_print;
}

/************************************
DEF
*/
/* Identificacion de una huella entre varias huellas */
int verify(struct fp_dev *dev, struct fp_print_data **data)
{
	int r;
	size_t offset = 0;
//Se hara siempre hasta que se encuentre que MATCH(hubo compatible) o NO MATCH (no compatible)
	do {
		struct fp_img *img = NULL;

		sleep(1);
		printf("\nScan your finger now.\n");
		/* IDENTIFY, de uno entre varios */
		r = fp_identify_finger(dev, data, &offset);

		printf("OFFSET; %d\n", offset);

		if (r < 0) {
			printf("verification failed with error %d :(\n", r);
			return r;
		}
		switch (r) {
		case FP_VERIFY_NO_MATCH:
			printf("NO MATCH!\n");
			return 0;
		case FP_VERIFY_MATCH:
			printf("MATCH!\n");
			return 1;
		case FP_VERIFY_RETRY:
			printf("Scan didn't quite work. Please try again.\n");
			break;
		case FP_VERIFY_RETRY_TOO_SHORT:
			printf("Swipe was too short, please try again.\n");
			break;
		case FP_VERIFY_RETRY_CENTER_FINGER:
			printf("Please center your finger on the sensor and try again.\n");
			break;
		case FP_VERIFY_RETRY_REMOVE_FINGER:
			printf("Please remove finger from the sensor and try again.\n");
			break;
		}
	} while (1);
}


typedef struct fp_user {
	  unsigned char buf;
		int user;
		int len;
} Users;

Users users[5];
/*funciona magica, todo*/
int save(struct fp_print_data *data, int user, struct fp_dev *dev,struct fp_print_data *data1,struct fp_print_data *data2){
	sqlite3 * db;
	char * sql;
	int rc;
	int len,len1,len2;
	int error;
	unsigned char *buf, *buf1,*buf2, *result;
	sqlite3_stmt *res;
	int n = 5; //Crear el arreglo de punteros exactamente con el numero de data que contendra
	//Se pide el tamano a la memoria
	struct fp_print_data ** fdata = (struct fp_print_data **) calloc(n,sizeof(struct fp_print_data *));

	// se transforman las dos datas, de dos huellas introducidas, a dos buffers
	len = fp_print_data_get_data(data, &buf);
	len1 = fp_print_data_get_data(data1, &buf1);
	len2 = fp_print_data_get_data(data2, &buf2);
//Se abre la BD
	rc = sqlite3_open("test.db", &db);
		if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(1);
    }
//SE inserta en la tabla, id y huella
	sql = "INSERT INTO Huellas (id,huella) VALUES (?,?), (?,?), (?,?)";
//Se prepara el statement
	rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
//Si el statement es correcto, introducimos en id y huella sus correspondiente valores
	if (rc == SQLITE_OK) {
		sqlite3_bind_int(res, 1, 4);
		sqlite3_bind_blob(res, 2, buf,len,0);
		sqlite3_bind_int(res, 3, 5);
		sqlite3_bind_blob(res, 4, buf1,len1,0);
		//
		sqlite3_bind_int(res, 5, 6);
		sqlite3_bind_blob(res, 6, buf2,len2,0);
	} else {
			fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
	}
	//Hacemos que funciona el statement
	int step = sqlite3_step(res);
	//Se finaliza el statement
	sqlite3_finalize(res);
	//Se cierra BD
	sqlite3_close(db);
/***************************************/

	//Se abre BD
	rc = sqlite3_open("test.db", &db);
	//Se hace un sql de select
	sql = "SELECT DISTINCT * FROM Huellas";
	//Preparacion de statement
	rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
	//Si statement correcto
/*	if (rc == SQLITE_OK) {
			sqlite3_bind_int(res, 1, 3);
	} else {

			fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
	}
*/
	 step = sqlite3_step(res);
	 int i = 0;
	while (i != 3) {
			printf("i:%d    %s: ",i, sqlite3_column_text(res, 0));
			printf("%s\n", sqlite3_column_text(res, 1));

			fdata[i] = fp_print_data_from_data((unsigned char *)sqlite3_column_blob(res, 1),len);
			i++;
	}
	//buf = sqlite3_column_blob(res, 1);

	//Anadimos en el arreglo de fdata (fp_print_data) las huellas leidas de la BD
	/*fdata[0] = fp_print_data_from_data(buf,len); //FUNCIONA
	fdata[1] = fp_print_data_from_data(buf1,len1);
	fdata[2] = fp_print_data_from_data(buf2,len2);*/
	//error = 0;
	//Se verifica (IDENTIFICA), mandandole el dispositivo y el arreglo de fdata
	do{
	  verify(dev,fdata);
	}while(1);

	sqlite3_finalize(res);
	sqlite3_close(db);
}


void getAll(){

}


int main(void)
{
	int r = 1, error = 0;
	struct fp_dscv_dev *ddev;
	struct fp_dscv_dev **discovered_devs;
	struct fp_dev *dev;
	struct fp_print_data *data, *data1,*data2;

	printf("This program will enroll your right index finger, "
		"unconditionally overwriting any right-index print that was enrolled "
		"previously. If you want to continue, press enter, otherwise hit "
		"Ctrl+C\n");
	getchar();

	r = fp_init();
	if (r < 0) {
		fprintf(stderr, "Failed to initialize libfprint\n");
		exit(1);
	}
	fp_set_debug(3);

	discovered_devs = fp_discover_devs();
	if (!discovered_devs) {
		fprintf(stderr, "Could not discover devices\n");
		goto out;
	}

	ddev = discover_device(discovered_devs);
	if (!ddev) {
		fprintf(stderr, "No devices detected.\n");
		goto out;
	}

	dev = fp_dev_open(ddev);
	fp_dscv_devs_free(discovered_devs);
	if (!dev) {
		fprintf(stderr, "Could not open device.\n");
		goto out;
	}

	printf("Opened device. It's now time to enroll your finger.\n\n");


	data = enroll(dev);
	if (!data)
		goto out_close;
//printf("Data: %s.\n\n",data->data);
	data1 = enroll(dev);
	if (!data1)
		goto out_close;

	data2 = enroll(dev);
		if (!data2)
			goto out_close;

	error = save(data,1,dev,data1,data2);
	if (error)
		goto out_close;

	//printf("Data saved in binary file.\n\n");



	r = fp_print_data_save(data, RIGHT_INDEX);
	if (r < 0)
		fprintf(stderr, "Data save failed, code %d\n", r);

	fp_print_data_free(data);
out_close:
	fp_dev_close(dev);
out:
	fp_exit();
	return r;
}
