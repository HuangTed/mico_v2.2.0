/**
******************************************************************************
* @file    user_main.c 
* @author  Eshen Wang
* @version V1.0.0
* @date    14-May-2015
* @brief   user main functons in user_main thread.
******************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
******************************************************************************
*/ 

#include "MICODefine.h"
#include "MicoFogCloud.h"
// for user
#include <Object.h>
#include <SysCom.h>
#include <MemMng.h>
#include <ObjDevice.h>
#include <ObjLights.h>
#include <ObjMusic.h>

#define user_log(M, ...) custom_log("USER", M, ##__VA_ARGS__)
#define user_log_trace() custom_log_trace("USER")


static mico_thread_t downstream_thread_handle = NULL;
static mico_thread_t upstream_thread_handle = NULL;
static mico_thread_t ObjectModule_thread_handle = NULL;
static mico_thread_t Music_thread_handle = NULL;

extern void upstream_thread(void* arg);
extern void downstream_thread(void* arg);
extern void ObjectModule_Thread(void* arg);
extern void Music_Thread(void* arg);


bool subscribe = true;
char track[16];
char url_path[16];



/* MICO user callback: Restore default configuration provided by user
* called when Easylink buttion long pressed
*/
void userRestoreDefault_callback(mico_Context_t *mico_context)
{
  //user_log("INFO: restore user configuration.");
}


/* user main function, called by AppFramework after FogCloud connected.
*/
OSStatus user_main( mico_Context_t * const mico_context )
{
  user_log_trace();
  OSStatus err = kUnknownErr;
  
  require(mico_context, exit);
  
  MemInit();
  SysComInit();
  ObjectInit();
  ObjDeviceInit();
  ObjLightsInit();
  ObjMusicInit();
  
  // create ObjectModule thread
  err = mico_rtos_create_thread(&ObjectModule_thread_handle,
                                MICO_APPLICATION_PRIORITY,
                                "ObjectModule",
                                ObjectModule_Thread,
                                STACK_SIZE_OBJECTMODULE_THREAD,
                                mico_context);
  require_noerr_action( err, exit, user_log("ERROR: Create ObjectModule thread failed!") );
  
  // Create Lights thread
  // err = mico_rtos_create_thread(&Lights_thread_handle,
                                // MICO_APPLICATION_PRIORITY,
                                // "Lights",
                                // Lights_Thread,
                                // STACK_SIZE_OBJECTMODULE_THREAD,
                                // mico_context);
  // require_noerr_action( err, exit, user_log("ERROR: Create Lights thread failed!") );
  
  // Create Music thread
  err = mico_rtos_create_thread(&Music_thread_handle,
                                MICO_APPLICATION_PRIORITY,
                                "Music",
                                Music_Thread,
                                STACK_SIZE_OBJECTMODULE_THREAD,
                                mico_context);
  require_noerr_action( err, exit, user_log("ERROR: Create Music thread failed!") );
  
  // start the downstream thread to handle user command
  err = mico_rtos_create_thread(&downstream_thread_handle, MICO_APPLICATION_PRIORITY, "downstream", 
                                downstream_thread, STACK_SIZE_DOWNSTREAM_THREAD, 
                                mico_context );
  require_noerr_action( err, exit, user_log("ERROR: create downstream thread failed!") );
    
  // start the upstream thread to upload temperature && humidity to user
  err = mico_rtos_create_thread(&upstream_thread_handle, MICO_APPLICATION_PRIORITY, "upstream", 
                                upstream_thread, STACK_SIZE_UPSTREAM_THREAD, 
                                mico_context );
  require_noerr_action( err, exit, user_log("ERROR: create uptream thread failed!") );
  
  user_log("user_main: start");
  while(1){

    mico_thread_sleep(10);
    user_log("user_main: Running");

    // test
    if(!MicoFogCloudIsConnect(mico_context)) {
        user_log("ERR > user_main: FogCloud disconnected");
    }
  }
  
exit:
  user_log("ERROR: user_main exit with err=%d", err);
  return err;
}
