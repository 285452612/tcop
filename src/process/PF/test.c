#include<stdio.h>
#include<string.h>

int main()
{
    FILE *fp =NULL;
    char file[256]={0};
    int len=0;
    char buf[2000];
    int i=-1;

    sprintf( file, "%s/test/testfile", getenv("HOME") );
#if 1 
    if((fp=fopen(file, "w+"))==NULL)
    {
        fprintf( stderr, "打开文件失败[%s]...", file );
        return -1;
    }

    len+=sprintf(buf+len,"%1000s\n"," ");
    len+=sprintf(buf+len,"bbbbbbbbbbbbbbbbbbbbbbb\n");
    fwrite(buf, len, 1, fp);
    //fprintf( fp, "%s", buf );
    fclose(fp);
#endif

    if((fp=fopen(file, "rb+"))==NULL)
    {
        fprintf( stderr, "打开文件失败[%s]...", file );
        return -1;
    }
    //fseek(fp, 0L, SEEK_SET);
    //memset(buf, 0, sizeof(buf));
    len=0;
    len+=sprintf(buf+len,"\n");
    len+=sprintf(buf+len,"hello              dddddddddddd\n");
    len+=sprintf(buf+len,"hello              dddddddddddd\n");
    len+=sprintf(buf+len,"hello              dddddddddddd\n");
    len+=sprintf(buf+len,"%20s\n","dddd");
    //rewind(fp);
    fwrite(buf, len, 1, fp);
    //fseek(fp, 0L, SEEK_SET);
    //fprintf( fp, "%s", buf );
    fflush(fp);
    fclose(fp);
}
