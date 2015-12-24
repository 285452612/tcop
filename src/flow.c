#include "flow.h"
#include "interface.h"

int initFlow()
{
    xmlDoc *flowDoc = NULL;
    char path[256] = {0};

    sprintf(path, "%s/conf/%d/flow.xml", OP_HOME, OP_BANKNODE);
    if (access(path, 0) != 0)
        sprintf(path, "%s/conf/opflow.xml", OP_HOME);

    flowDoc = xmlRecoverFile(path);
    returnIfNullLoger(flowDoc, -1, "��ʼ�����������ļ�ʧ�� file:%s", path);

    //init OP_FLOW

    return 0;
}

int callFlow(const char *nodename)
{
    return 0;
}
