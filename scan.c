#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "esp_spi_flash.h"
#include "rom/spi_flash.h"
#include <stdio.h>
#include <stdlib.h>


#include "esp_ota_ops.h"
//#include "espcurl.h"
#include "cJSON.h"

int state = 0;



#define APP_VERTION "Factory(Default)"
#define EXAMPLE_WIFI_SSID "Valera111_ESP32_AP"
#define EXAMPLE_WIFI_PASS ""





static const char *TAG = "ota";
#define SLOW_BLINKING      1000
#define SPRINT_BLINKING    100
#define JUST_LIGHTING      10


bool Sosket_Status = false;
int blinking_delay = SLOW_BLINKING;

//string  str = "ffsdfd000";

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
//const static char http_index_hml[] = "<html><body><form method=get><label> </label><br><label> <form method=\"POST\" enctype=\"multipart/form-data\"> <input name=\"user-file\" type=\"file\"> <br><br> <input type=\"submit\" value=\"Update\">   </label><br><label>SSID</label><br><input  type='text' name='ssid' maxlength='30' size='15'><br><label>Password</label><br><input  type='password' name='password' maxlength='30' size='15'><br>  <br><input  type='submit' value='connect' </form></body></html>";
const static char http_index_hml[] = "<html><body><form method=get>            <h1 style=\"text-align: center;\"><span style=\"color:#FF0000\"><strong>ESP32 Controll Server</strong></span></h1>           <label></label> <br><label>Firmware version: </label>" APP_VERTION "         <p><input name=\"Update\" type=\"button\" value=\"Update\" /></p>            <br><label></label> <br><label>SSID</label><br><input  type='text' name='ssid' maxlength='30' size='15'><br><label>Password</label><br><input  type='password' name='password' maxlength='30' size='15'><br><br><input  type='submit' value='connect' > </form></body></html>";












//const static char http_index_hml[] = "<html><body><form method=get><label> </label><br><label> </label><br><label>SSID</label><br><input  type='text' name='ssid' maxlength='30' size='15'><br><label>Password</label><br><input  type='password' name='password' maxlength='30' size='15'><br><button style=\"color:blue; id=\"111\" class=\"led\">CONNECT</button> </form></body></html>";
//<form method=\"POST\" enctype=\"multipart/form-data\"> <input name=\"user-file\" type=\"file\"> <br><br> <input type=\"submit\" value=\"Отправить\">  </form>
//const static char http_index_hml[] = "<form action=\"data.php\"><input type=\"text\" name=\"field1\"><input type=\"submit\" value=\"SEND\"></form><?echo $_GET['field1'];?>";


/*const static char http_index_hml[] =
"<html>"
" <head>"
" <script type=\"text/javascript\">"
"    function send(onOff) {"
"      localStorage['accesskey'] = document.getElementById('accesskey').value;"
"      var xmlhttp = new XMLHttpRequest();  ";
"      xmlhttp.open('POST', \"http://\" + window.location.hostname + '/api/gpio/write', true);"
"      xmlhttp.setRequestHeader(\"Authorization\", \"Bearer \" + localStorage['accesskey']);"
"      xmlhttp.onreadystatechange = function() {"
"        if(xmlhttp.readyState == 4){"
"          if(xmlhttp.status < 200 || xmlhttp.status > 299) {"
"            alert('ERROR: Command return ' + xmlhttp.status + ' ' + xmlhttp.responseText, true);"
"          }"
"        }"
"      }"
"     var json = {}"
"     json[\"5\"] = onOff ? 1 : 0;"
"     xmlhttp.send(JSON.stringify(json));"
"   }"
"   function init() {"
"     document.getElementById('accesskey').value = localStorage['accesskey'];"
"   }"
" </script>"
" </head>"
" <body onload=\"init()\">"
"  <label>AccesKey: </label><input type=\"password\" id=\"accesskey\"><br><br>"
"  <input type=\"button\" value="On" onclick=\"send(true);\">   <input type=\"button\" value=\"Off\" onclick=\"send(false);\">"
" </body>"
"</html>"
;
*/



















const static char http_html_hdr_2[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_index_hml_2[] = "<html><head><title>Control</title></head><body><h1>Control</h1><a href=\"h\">On</a><br><a href=\"l\">Off</a></body></html>";

void wifi_reset_to_point_mode();
esp_err_t print_saved_ssid(void); //От Вани

char buffer[32];
char SSID_mem[32];
char PASS_mem[64];
esp_err_t print_save_hub_status(char *hubStatus); // мое
esp_err_t print_read_hub_status(char *hubStatus); // мое
esp_err_t print_save_hub_SSID_PASS(char *, char *);//мое



#define EXAMPLE_WIFI_SSID_2 "SenseSystems"
#define EXAMPLE_WIFI_PASS_2 "1111"






#define POINT            "VELVEL"
#define POINT_PASS       ""
#define CLIENT           "SenseSystems"
#define CLIENT_PASS      "1111"



char HUB_STATUS_1[32] = "SSID_PASS";
char HUB_STATUS_2[64] = "noSSID_noPASS";
char WIFI_SSID_2[32] = "Valera_ESP32_AP";
char WIFI_PASS_2[64] = "";

char ssid[32] = "";
char pass[64] = "";
bool buttotReset = false;


int attampts_Number = 0;

const int DIODE_PIN = 5;

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;



static uint32_t g_wbuf[SPI_FLASH_SEC_SIZE / 4];
static uint32_t g_rbuf[SPI_FLASH_SEC_SIZE / 4];






void readWriteTask(void *pvParameters)
{
    srand(0);

    for (uint32_t base_addr = 0x200000;
         base_addr < 0x300000;
         base_addr += SPI_FLASH_SEC_SIZE)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        printf("erasing sector %x\n", base_addr / SPI_FLASH_SEC_SIZE);
        spi_flash_erase_sector(base_addr / SPI_FLASH_SEC_SIZE);

        for (int i = 0; i < sizeof(g_wbuf)/sizeof(g_wbuf[0]); ++i) {
            g_wbuf[i] = rand();
        }

        printf("writing at %x\n", base_addr);
        spi_flash_write(base_addr, g_wbuf, sizeof(g_wbuf));

        memset(g_rbuf, 0, sizeof(g_rbuf));
        printf("reading at %x\n", base_addr);
        spi_flash_read(base_addr, g_rbuf, sizeof(g_rbuf));
        for (int i = 0; i < sizeof(g_rbuf)/sizeof(g_rbuf[0]); ++i) {
            if (g_rbuf[i] != g_wbuf[i]) {
                printf("failed writing or reading at %d\n", base_addr + i * 4);
                printf("got %08x, expected %08x\n", g_rbuf[i], g_wbuf[i]);
                return;
            }
        }

        printf("done at %x\n", base_addr);
        base_addr += SPI_FLASH_SEC_SIZE;
    }

    printf("read/write/erase test done\n");
    vTaskDelete(NULL);
}












static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
    	blinking_delay = SLOW_BLINKING;
    	if(Sosket_Status == false)
    	printf("2---------------Connectig(new_SSID_PASS)...--------------2\n");
    	else
    	printf("3-----------Connecting...------------3\n");
    	//if(Sosket_Status == true)
    		ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    case SYSTEM_EVENT_AP_START:
    	Sosket_Status = false;
    	blinking_delay = SPRINT_BLINKING ;
    	break;
    case SYSTEM_EVENT_STA_GOT_IP:
    	if(Sosket_Status == false)
    	{Sosket_Status = true;
    	print_save_hub_SSID_PASS(ssid, pass);
    	print_save_hub_status( HUB_STATUS_1);
    	}
    	blinking_delay = JUST_LIGHTING ;

        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
    	blinking_delay = JUST_LIGHTING ;
    	break;
    case SYSTEM_EVENT_STA_DISCONNECTED:

    	blinking_delay = SLOW_BLINKING;
    	if(buttotReset == true)
    		{buttotReset = false;
    		break;}
    	if(Sosket_Status == true)
    		{
    		ESP_ERROR_CHECK(esp_wifi_connect());
    		printf("1----------------Connecting(disconnected)...----------------1\n");
    		xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    		}
    	if(Sosket_Status == false)
    		{
    		esp_wifi_connect();
    		printf("2---------------Connectig(new_SSID_PASS)...--------------2\n");
    		if(attampts_Number<5)
    		{
    		attampts_Number ++;
    	    }
    		else
    			{
    			attampts_Number = 0;
    			wifi_reset_to_point_mode();
    			}
    		xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    		}
        break;

    default:
        break;
    }
    return ESP_OK;
}




static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
			.channel=0,
			.authmode=WIFI_AUTH_OPEN,
			.ssid_hidden=0,
			.max_connection=4,
			.beacon_interval=100
        },
    };
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}


static void initialise_wifi_2(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

    wifi_config_t wifi_config = {
            .sta = {
                .ssid = EXAMPLE_WIFI_SSID_2,
                .password = EXAMPLE_WIFI_PASS_2,
            },
        };
    strncpy((char*) wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid)); //!!!
    strncpy((char*) wifi_config.ap.password, pass, sizeof(wifi_config.ap.password));//!!!
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}


void wifi_reset_to_klient_mode(char *ssid, char *pass)
{

	 //Sosket_Status=true;
	 wifi_config_t wifi_config = {
	            .sta = {
	                .ssid = EXAMPLE_WIFI_SSID_2,
	                .password = EXAMPLE_WIFI_PASS_2,
	            },
	        };

	    strncpy((char*) wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid)); //!!!
	    strncpy((char*) wifi_config.ap.password, pass, sizeof(wifi_config.ap.password));//!!!
	    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
	    ESP_ERROR_CHECK( esp_wifi_start() );

}




void wifi_reset_to_point_mode()
{

	 //Sosket_Status=false;
	 wifi_config_t wifi_config = {
	        .ap = {
	            .ssid = EXAMPLE_WIFI_SSID,
	            .password = EXAMPLE_WIFI_PASS,
				.channel=0,
				.authmode=WIFI_AUTH_OPEN,
				.ssid_hidden=0,
				.max_connection=4,
				.beacon_interval=100
	        },
	    };
	    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
	    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &wifi_config) );
	    ESP_ERROR_CHECK( esp_wifi_start() );
}







void wifi_reset_to_PointClient_mode(char *ssid, char *pass)
{
	tcpip_adapter_init();
	    wifi_event_group = xEventGroupCreate();
	    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
	    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );


	    wifi_config_t wifi_config = {
	           .ap = {
	               .ssid = CLIENT,
	               .password = CLIENT_PASS,
	   			.channel=0,
	   			.authmode=WIFI_AUTH_OPEN,
	   			.ssid_hidden=0,
	   			.max_connection=4,
	   			.beacon_interval=100
	           },
			   .sta ={
			   .ssid = POINT,
			   .password = POINT_PASS,},
	       };




	//strncpy((char*) wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid)); //!!!
    //strncpy((char*) wifi_config.ap.password, pass, sizeof(wifi_config.ap.password));//!!!
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) );
	ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &wifi_config) );
	ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
	ESP_ERROR_CHECK( esp_wifi_start());

}







static void
http_server_netconn_serve(struct netconn *conn)
{
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;
  int pp = 0;
  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);






  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);

    /*--------------MMMMMMMMMMMMMMMMMMMMMMM---------------*/
    printf(buf);


    char ssid_mask[]="ssid=";
    char pass_mask[]="password=";
    int pointer = -1;



    ///////////////////////

    for(int i=0;i<600;i++)
    {
    	int man = 0;
    	for(int j=0;j<5;j++)
    		{
    		if(buf[i+j]==ssid_mask[j])
    			man++;
    		}
    	if(man==5)
    	{	pointer = i;
    	break;}

    	else
    		pointer = -1;
    }

    int i = 0;
    if (pointer>=0)
    {

    	do
    	{
    	if(buf[pointer+i+5]=='&')
    			break;

    	ssid[i] = buf[pointer+i+5];
    	i++;
    	} while (buf[pointer+i+5]!='&');

    	if (i>5) pp = 1;
    }
    ssid[i]='\0';

    ////////////////////////////

    pointer = -1;

    /////////////////////////////

    for(int i=0;i<40;i++)
        {
        	int man = 0;
        	for(int j=0;j<9;j++)
        		{
        		if(buf[i+j]==pass_mask[j])
        			man++;
        		}
        	if(man==9)
        	{	pointer = i;
        	break;}

        	else
        		pointer = -1;

        }
        i = 0;
        if (pointer>=0)
        {
        	do
        	{
        		if(buf[pointer+i+9]==' ')
        		break;

        		pass[i] = buf[pointer+i+9];
        	i++;
        	} while (buf[pointer+i+9]!=' ');
        }
     pass[i]='\0';
     //////////////////////////////////
        int j = 0;


    /*--------------MMMMMMMMMMMMMMMMMMMMMMM---------------*/

    /* Is this an HTTP GET command? (only check the first 5 chars, since
    there are other formats for GET, and we're keeping it very simple )*/
    if (buflen>=5 &&
        buf[0]=='G' &&
        buf[1]=='E' &&
        buf[2]=='T' &&
        buf[3]==' ' &&
        buf[4]=='/' ) {
        // printf("%c\n", buf[5]);
      /* Send the HTML header
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
       */
       gpio_pad_select_gpio(DIODE_PIN);
       /* Set the GPIO as a push/pull output */
       gpio_set_direction(DIODE_PIN, GPIO_MODE_OUTPUT);
       if(buf[5]=='h'){
         gpio_set_level(DIODE_PIN,1);

       }
       if(buf[5]=='l'){
         gpio_set_level(DIODE_PIN,0);

       }
      netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);

      /* Send our HTML page */
      netconn_write(conn, http_index_hml, sizeof(http_index_hml)-1, NETCONN_NOCOPY);
    }

  }
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);

  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);


  if (pp==1)
      {
	  attampts_Number = 0;
	  wifi_reset_to_klient_mode(ssid, pass);
	 // wifi_reset_to_PointClient_mode(ssid, pass);


      }


}



//WiFi розетка начало

static void
http_server_netconn_serve_2(struct netconn *conn)
{
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;

  // Read the data from the port, blocking if nothing yet there.
  // We assume the request (the part we care about) is in one netbuf
  err = netconn_recv(conn, &inbuf);

  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);
    //printf(buf);


    //--------------MMMMMMMMMMMMMMMMMMMMMMM---------------

    //Is this an HTTP GET command? (only check the first 5 chars, since
    //there are other formats for GET, and we're keeping it very simple )
    if (buflen>=5 &&
        buf[0]=='G' &&
        buf[1]=='E' &&
        buf[2]=='T' &&
        buf[3]==' ' &&
        buf[4]=='/' ) {
        //  printf("%c\n", buf[5]);
      // Send the HTML header
      //       * subtract 1 from the size, since we dont send the \0 in the string
      //       * NETCONN_NOCOPY: our data is const static, so no need to copy it
      //
       gpio_pad_select_gpio(DIODE_PIN);
       // Set the GPIO as a push/pull output
       gpio_set_direction(DIODE_PIN, GPIO_MODE_OUTPUT);
       if(buf[5]=='h'){
         gpio_set_level(DIODE_PIN,1);

       }
       if(buf[5]=='l'){
         gpio_set_level(DIODE_PIN,0);

       }
      netconn_write(conn, http_html_hdr_2, sizeof(http_html_hdr_2)-1, NETCONN_NOCOPY);

      // Send our HTML page
      netconn_write(conn, http_index_hml_2, sizeof(http_index_hml_2)-1, NETCONN_NOCOPY);


    }

  }
  // Close the connection (server closes in HTTP)
  netconn_close(conn);

  // Delete the buffer (netconn_recv gives us ownership,
  // so we have to make sure to deallocate the buffer)
  netbuf_delete(inbuf);
}


//WiFi розетка конец


static void http_server(void *pvParameters)
{
  struct netconn *conn, *newconn;
  err_t err;
  conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, NULL, 80);
  netconn_listen(conn);
  while(1)
{
  do {
     err = netconn_accept(conn, &newconn);
     if (err == ERR_OK) {
    	 if(Sosket_Status==false)
    	     	{
    		 http_server_netconn_serve(newconn);

    	     	}
    	 if(Sosket_Status==true)
    	     	     	{
    		 http_server_netconn_serve_2(newconn);
    	     	     	}
       netconn_delete(newconn);
     }
   } while(err == ERR_OK);
   netconn_close(conn);
   netconn_delete(conn);
}
}

static void diode_blinking(void *pvParameters)
{

	gpio_pad_select_gpio(17);
	gpio_set_direction(17, GPIO_MODE_OUTPUT);
	gpio_set_level(17,1);


	while(1)
	{
		/*if(blinking_delay>0)
		{*/
		gpio_set_level(17,1);
		vTaskDelay(blinking_delay / portTICK_PERIOD_MS);
		gpio_set_level(17,0);
		vTaskDelay(blinking_delay / portTICK_PERIOD_MS);
		/*}
		else
		{
			gpio_set_level(17,1);
			vTaskDelay(10 / portTICK_PERIOD_MS);
			gpio_set_level(17,0);
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}*/
	}
}




static void push_button(void *pvParameters)
{
	gpio_pad_select_gpio(2);
	gpio_pullup_dis(2);
	gpio_set_direction(2, GPIO_MODE_INPUT );
	int pin_condition = 0;


	while(1)
	{
		pin_condition = gpio_get_level(2);
		if(pin_condition)
		{
			for(int i=0; i<10; i++)
							{
			pin_condition = gpio_get_level(2);
			vTaskDelay(100 / portTICK_PERIOD_MS);//vanya
							}
			pin_condition = gpio_get_level(2);

			if(pin_condition)
			{
			attampts_Number = 0;
		   /* print_read_hub_status(buffer); // мое
		    print_save_hub_status( HUB_STATUS_1); //мое
		    print_save_hub_SSID_PASS(WIFI_SSID_2, WIFI_PASS_2);//мое*/
			buttotReset = true;
			print_save_hub_status( HUB_STATUS_2);
		    wifi_reset_to_point_mode();
			}
		}
		vTaskDelay(100 / portTICK_PERIOD_MS);//vanya
	}
}





/*------От Вани работа с памятью-------*/

esp_err_t print_saved_ssid(void)
{
    nvs_handle wifi_handle;
    esp_err_t err;

	// Open NVS
    printf("\n Opening NVS...\n");
    err = nvs_open("storage", NVS_READWRITE, &wifi_handle);
    if (err != ESP_OK) return err;


	// Read SSID written on NVS
	size_t required_size = 0;  // value will default to 0, if not set yet in NVS
	// obtain required memory space to store blob being read from NVS
	err = nvs_get_str(wifi_handle, "ssid_nvs", NULL, &required_size); // проверяем записано ли туда чтото
	printf("Obtain required memory space to store complex data structure (ssid) being read from NVS...\n");



	if (required_size == 0) {  //ssid ще не записаний, записуэмо
		printf("\n No SSID hasnt been saved yet!");
		printf("\n Saving  random SSID...");


		char WIFI_SSID_2[32] = "Postirayka2";

		err = nvs_set_str(wifi_handle, "ssid_nvs", WIFI_SSID_2); //

		printf("\n Writing ssid to  NVS...\n");

		if (err != ESP_OK) return err;
		// Commit. Mandatory!
		err = nvs_commit(wifi_handle);

		printf("\n Committing to  NVS...\n");

		if (err != ESP_OK) return err;

	}
	else { //якщо значення раніше було записане, то вивести його на екран

		char* ssid_retrieved = malloc(required_size);
		err = nvs_get_str(wifi_handle, "ssid_nvs", ssid_retrieved, &required_size);
		printf(" Getting existing ssid...\n");
		if (err != ESP_OK) return err;

		//Print OUT saved SSID

		printf(" \n Current SSID is : %s \n", ssid_retrieved);

		free(ssid_retrieved); //free allocated memory by means of malloc()
	}

	    // Close NVS
    nvs_close(wifi_handle);
    return ESP_OK;

}


/*------конец От Вани работа с памятью-------*/



esp_err_t print_read_hub_status(char *hubStatus)
{
		nvs_handle wifi_handle;
	    esp_err_t err;

		// Open NVS
	    printf("\n Opening NVS...\n");
	    err = nvs_open("storage", NVS_READWRITE, &wifi_handle);
	    if (err != ESP_OK) return err;


		// Read SSID written on NVS
		size_t required_size = 0;  // value will default to 0, if not set yet in NVS
		// obtain required memory space to store blob being read from NVS
		err = nvs_get_str(wifi_handle, "hubStatus", NULL, &required_size); // проверяем записано ли туда чтото
		printf("Obtain required memory space to store complex data structure (hubStatus) being read from NVS...\n");



		if (required_size == 0) {  //ssid ще не записаний, записуэмо
			printf("\n No  hubStatus been saved yet!");
			printf("\n Saving  hubStatus...");


			char HUB_STATUS_2[32] = "noSSID_noPASS";

			err = nvs_set_str(wifi_handle, "hubStatus", HUB_STATUS_2); //

			printf("\n Writing hubStatus to  NVS...\n");

			if (err != ESP_OK) return err;
			// Commit. Mandatory!
			err = nvs_commit(wifi_handle);

			printf("\n Committing to  NVS...\n");

			if (err != ESP_OK) return err;

		}
		else { //якщо значення раніше було записане, то взяти
							err = nvs_get_str(wifi_handle, "hubStatus", hubStatus, &required_size);
							printf(" Getting existing hubStatus...\n");
							if (err != ESP_OK) return err;
							printf(" \n Current hubStatus is : %s \n", hubStatus);

			 }

	nvs_close(wifi_handle);
	return ESP_OK;
}





esp_err_t print_save_hub_status(char *hubStatus)
{

		nvs_handle wifi_handle;
	    esp_err_t err;

		// Open NVS
	    printf("\n Opening NVS...\n");
	    err = nvs_open("storage", NVS_READWRITE, &wifi_handle);
	    if (err != ESP_OK) return err;


		// Read SSID written on NVS
		size_t required_size = 0;  // value will default to 0, if not set yet in NVS
		// obtain required memory space to store blob being read from NVS
		err = nvs_get_str(wifi_handle, "hubStatus", NULL, &required_size); // проверяем записано ли туда чтото
		printf("Obtain required memory space to store complex data structure (hubStatus) being read from NVS...\n");



		if (required_size == 0) {  //ssid ще не записаний, записуэмо
			printf("\n No  hubStatus been saved yet!");
			printf("\n Saving  hubStatus...");


			char HUB_STATUS_2[32] = "noSSID_noPASS";

			err = nvs_set_str(wifi_handle, "hubStatus", HUB_STATUS_2); //

			printf("\n Writing hubStatus to  NVS...\n");

			if (err != ESP_OK) return err;
			// Commit. Mandatory!
			err = nvs_commit(wifi_handle);

			printf("\n Committing to  NVS...\n");

			if (err != ESP_OK) return err;

		}

			else { //якщо значення раніше було записане, то вивести його на екран

				            err = nvs_set_str(wifi_handle, "hubStatus", hubStatus); //
							printf("\n Writing hubStatus to  NVS...\n");
							if (err != ESP_OK) return err;
							// Commit. Mandatory!
							err = nvs_commit(wifi_handle);

							printf("\n Committing to  NVS...\n");

							if (err != ESP_OK) return err;
				 }

		 nvs_close(wifi_handle);
		 return ESP_OK;
}












esp_err_t print_save_hub_SSID_PASS(char *SSID, char *PASS)
{
			nvs_handle wifi_handle;
		    esp_err_t err;

			// Open NVS
		    printf("\n Opening NVS...\n");
		    err = nvs_open("storage", NVS_READWRITE, &wifi_handle);
		    if (err != ESP_OK) return err;


			// Read SSID written on NVS
			size_t required_size_SSID = 0;  // value will default to 0, if not set yet in NVS
			size_t required_size_PASS = 0;

			err = nvs_get_str(wifi_handle, "SSID", NULL, &required_size_SSID); // проверяем записано ли туда чтото
			err = nvs_get_str(wifi_handle, "PASS", NULL, &required_size_PASS);
			printf("Obtain required memory space to store complex data structure (hubStatus) being read from NVS...\n");



			if (required_size_SSID  == 0) {  //ssid ще не записаний, записуэмо

				char ssid_2[32] = "---";
				err = nvs_set_str(wifi_handle, "SSID", ssid_2); //
				printf("\n Writing ssid_2 to  NVS...\n");
				if (err != ESP_OK) return err;
				err = nvs_commit(wifi_handle);
				printf("\n Committing to  NVS...\n");
				if (err != ESP_OK) return err;
			}

			if (required_size_PASS  == 0) {  //ssid ще не записаний, записуэмо

				char pass_2[64] = "---";
				err = nvs_set_str(wifi_handle, "PASS", pass_2); //
				printf("\n Writing pass_2 to  NVS...\n");
				if (err != ESP_OK) return err;
				err = nvs_commit(wifi_handle);
			    printf("\n Committing to  NVS...\n");
				if (err != ESP_OK) return err;
						}

			if (required_size_SSID  > 0) {

			    err = nvs_set_str(wifi_handle, "SSID", SSID); //
			    printf("\n Reading SSID from  NVS...\n");
				err = nvs_commit(wifi_handle);
				printf("\n Committing to  NVS...\n");
				if (err != ESP_OK) return err;
			}

			if (required_size_PASS  > 0) {

				err = nvs_set_str(wifi_handle, "PASS", PASS); //
				printf("\n Reading PASS from  NVS...\n");
				err = nvs_commit(wifi_handle);
				printf("\n Committing to  NVS...\n");
				if (err != ESP_OK) return err;
			}

			 nvs_close(wifi_handle);
			 return ESP_OK;
}






esp_err_t print_read_hub_SSID_PASS(char *SSID, char *PASS)
{

	nvs_handle wifi_handle;
			    esp_err_t err;

				// Open NVS
			    printf("\n Opening NVS...\n");
			    err = nvs_open("storage", NVS_READWRITE, &wifi_handle);
			    if (err != ESP_OK) return err;


				// Read SSID written on NVS
				size_t required_size_SSID = 0;  // value will default to 0, if not set yet in NVS
				size_t required_size_PASS = 0;

				err = nvs_get_str(wifi_handle, "SSID", NULL, &required_size_SSID); // проверяем записано ли туда чтото
				err = nvs_get_str(wifi_handle, "PASS", NULL, &required_size_PASS);
				printf("Obtain required memory space to store complex data structure (hubStatus) being read from NVS...\n");



				if (required_size_SSID  == 0) {  //ssid ще не записаний, записуэмо

					char ssid_2[32] = "---";
					err = nvs_set_str(wifi_handle, "SSID", ssid_2); //
					printf("\n Writing ssid_2 to  NVS...\n");
					if (err != ESP_OK) return err;
					err = nvs_commit(wifi_handle);
					printf("\n Committing to  NVS...\n");
					if (err != ESP_OK) return err;
				}

				if (required_size_PASS  == 0) {  //ssid ще не записаний, записуэмо

					char pass_2[64] = "---";
					err = nvs_set_str(wifi_handle, "PASS", pass_2); //
					printf("\n Writing pass_2 to  NVS...\n");
					if (err != ESP_OK) return err;
					err = nvs_commit(wifi_handle);
				    printf("\n Committing to  NVS...\n");
					if (err != ESP_OK) return err;
							}


				if(required_size_SSID > 0) {

												err = nvs_get_str(wifi_handle, "SSID", SSID, &required_size_SSID);
												printf(" Getting existing SSID...\n");
												if (err != ESP_OK) return err;
												printf(" \n Current SSID is : %s \n", SSID);


												}

				if(required_size_PASS > 0) {

												err = nvs_get_str(wifi_handle, "PASS", PASS, &required_size_PASS);
												printf(" Getting existing PASS...\n");
												if (err != ESP_OK) return err;
												printf(" \n Current PASS is : %s \n", PASS);

												}

						 nvs_close(wifi_handle);
						 return ESP_OK;
}










void app_main()
{

	/*esp_rom_spiflash_config_param(0x1540ef, 4*1024*1024, 64*1024, 4096, 256, 0xffff); //esp_rom_spiflash_config_param
	 xTaskCreatePinnedToCore(&readWriteTask, "readWriteTask", 2048, NULL, 5, NULL, 0);*/

	 esp_err_t err = nvs_flash_init();
	    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
	        ESP_ERROR_CHECK(nvs_flash_erase());
	        err = nvs_flash_init();
	    }

	//nvs_flash_init();



    system_init();
    print_read_hub_status(buffer);

    if(buffer[0]=='n')
    	{
    	Sosket_Status=false;
    	}

    if(buffer[0]=='S')
        {
    	Sosket_Status=true;
    	print_read_hub_SSID_PASS(ssid, pass);
        }


    if(Sosket_Status==false)
    	{
    	initialise_wifi();
    	}
    if(Sosket_Status==true)
        {
        initialise_wifi_2();
        }

    xTaskCreate(&http_server, "http_server", 2048, NULL, 5, NULL);
    //xTaskCreate(&diode_blinking, "diode_blinking", 2048, NULL, 5, NULL);
    xTaskCreate(&push_button, "push_button", 2048, NULL, 5, NULL); // reset button to AP mode
//
}



