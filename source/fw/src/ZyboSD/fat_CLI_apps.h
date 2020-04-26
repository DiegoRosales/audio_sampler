

#ifndef FAT_CLI_APPS_H
#define FAT_CLI_APPS_H


#define mainSD_CARD_DISK_NAME "/"
#define cliNEW_LINE "\n\r"

void file_to_buffer( FF_FILE *pxFile, uint8_t *buffer, size_t buffer_len );
size_t load_file_to_memory( char * file_name, uint8_t *buffer, size_t buffer_len );
size_t load_file_to_memory_malloc( char *file_name, uint8_t ** buffer, size_t max_buffer_len, size_t overhead );

void register_fat_cli_commands( void );

#endif