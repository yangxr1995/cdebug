target_link_options(${PROJECT_NAME}
    PUBLIC    
 -Wl,--wrap=strcmp 
 -Wl,--wrap=strncmp  
 -Wl,--wrap=strcpy 
 -Wl,--wrap=strncpy  
 -Wl,--wrap=strstr  
 -Wl,--wrap=strcasestr  
 -Wl,--wrap=open -Wl,--wrap=close  
 -Wl,--wrap=fopen -Wl,--wrap=fclose  
 -Wl,--wrap=popen -Wl,--wrap=pclose  
 -Wl,--wrap=read -Wl,--wrap=write  
 -Wl,--wrap=fread -Wl,--wrap=fwrite  
 -Wl,--wrap=system   
 -Wl,--wrap=sleep  
 -Wl,--wrap=pthread_create  
 -Wl,--wrap=execv  
 -Wl,--wrap=fgets  
 -Wl,--wrap=unlink  
 -Wl,--wrap=access  
 -Wl,--wrap=select  
 -Wl,--wrap=epoll_wait  
 -Wl,--wrap=wait  
 -Wl,--wrap=waitpid  
 -Wl,--wrap=socket  
 -Wl,--wrap=bind  
 -Wl,--wrap=listen  
 -Wl,--wrap=connect  
 -Wl,--wrap=send  
 -Wl,--wrap=sendto  
 -Wl,--wrap=recv  
 -Wl,--wrap=recvfrom  
 -Wl,--wrap=recvmsg  

 -Wl,--wrap=epoll_create 
 -Wl,--wrap=epoll_create1
 -Wl,--wrap=epoll_ctl
 -Wl,--wrap=epoll_wait
 -Wl,--wrap=epoll_pwait

 -Wl,--wrap=pthread_mutex_lock
 -Wl,--wrap=pthread_mutex_trylock
 -Wl,--wrap=pthread_mutex_unlock
)
