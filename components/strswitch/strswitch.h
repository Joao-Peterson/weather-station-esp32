#ifndef _STRSWITCH_HEADER_
#define _STRSWITCH_HEADER_

// Example:
//     sswitch(argv[1]){
//         scase("admin")
//             printf("\nHello admin\n");
//         sbreak;

//         scase("pass")
//             printf("\nlogin\n");
//         sbreak;

//         sdefault
//             printf("\ninput: %s\n", argv[1]);
//         sbreak;
//     }
#define sswitch(str) char *_strswitchstrtocmp = (char*)str;
#define scase(comp_str) if(!strcmp(comp_str, _strswitchstrtocmp)){
#define sdefault {
#define sbreak }

#endif