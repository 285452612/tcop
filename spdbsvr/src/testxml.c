#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"
#include "udb.h"

// ����һ��xml�ļ�
xmlDocPtr getdoc(char *docname)
{
    xmlDocPtr doc;
    doc = xmlParseFile(docname);
    if (doc == NULL )
    {
        fprintf(stderr,"Document not parsed successfully. \n");
        exit(-1);
    }
    return doc;
}

// ���Խڵ��滻
int TestReplaceNode(xmlDocPtr dest, xmlDocPtr src, const char *xpath)
{
    xmlNodePtr dest_node, src_node;

    if ((dest_node = XmlLocateNode(dest, xpath)) == NULL)
        return -1;
    if ((src_node = XmlLocateNode(src, xpath)) == NULL)
        return -1;
    // ע: ������xmlCopyNode, src�е�src_node�ڵ㽫��ɾ��
    if ( xmlReplaceNode(dest_node, xmlCopyNode(src_node, 1)) == NULL )
        return -1;

    return 0;
}

// ��ǰ�ڵ�����һ��������
int TestAddNodeProp(xmlNodePtr pNode, char *pcName, char *pcVal)
{
    xmlChar *pEncName;
    xmlChar *pEncVal;

    pEncName = ConvIn(pcName);
    if ( pEncName == NULL )
        return -1;

    pEncVal = ConvIn(pcVal);
    if ( pEncVal == NULL )
    {
        free( pEncName );
        return -1;
    }

    if ( xmlNewProp( pNode, pEncName, pEncVal ) == NULL )
    {
        free( pEncName );
        free( pEncVal );
        return -1;
    }

    free( pEncName );
    free( pEncVal );

    return 0;
}

void test1(char *docname)
{
    xmlDocPtr doc;
    xmlXPathObjectPtr result;
    xmlNodePtr extradata = NULL;
    xmlNodePtr node = NULL;
    char tmp[512];
    int i;

    // ����һ��xml�ļ�
    doc = xmlParseFile(docname);

    // ���ҽڵ㼯TrnDetail
    result = getnodeset(doc, "/UFTP/TrnDetail");
    if (result->nodesetval->nodeNr <= 0)
    {
        printf("nodeset not found\n");
        exit(-1);
    }

    for (i = 0; i < result->nodesetval->nodeNr; i++)
    {
        fprintf(stdout, "�� %d ����¼:\n", i+1);
        fprintf(stdout, "-----------------------------------------\n");

        // �ƶ��ڵ㵽Payer
        node = XmlFetchNode(result->nodesetval->nodeTab[i], "Payer");
        // ʵ��Ӧ����Ӧ���ͷ�ConvOut����ڴ�
        printf("Payer = [%s]\n", XmlNodeGetContent(node));

        XmlGetNodeString(result->nodesetval->nodeTab[i], "Payer", tmp, sizeof(tmp));
        printf("Payer = [%s]\n", tmp);

        // �ƶ��ڵ㵽ExtraData
        extradata = XmlFetchNode(result->nodesetval->nodeTab[i], "ExtraData");
        // �ƶ��ڵ㵽ExtraData/aa
        node = XmlFetchNode(extradata, "aa");
        printf("ExtraData/aa = [%s]\n", ConvOut(xmlNodeGetContent(node)));
        XmlSetNodeString(extradata, "aa", "aaaaaaaaaa");
        XmlSetNodeString(extradata, "newnode", "�µĽڵ�����");
        node = XmlFetchNode(extradata, "kk");
        printf("ExtraData/kk = [%s]\n", ConvOut(xmlNodeGetContent(node)));
        printf("\n");
    }

    xmlSaveFile("set.xml", doc);
    xmlXPathFreeObject(result);
    xmlFreeDoc(doc);

    // ���libxml�������ú�ռ�õ��ڴ�
    xmlCleanupParser();

    printf("test1 ����!\n");
    return;
}

void test2(char *buf)
{
    xmlDocPtr doc;
    xmlChar *xmlBuf;
    int len;
    char tmp[1024];

    // ����xml��ʽ���ַ���
    printf("�����ַ���:\n%s\n", buf);
    doc = xmlParseMemory(buf, strlen(buf));

    // ȡ TrnCode
    printf("----------------------\n");
    XmlGetString(doc, "//MsgHdrRq/TrnCode", tmp, sizeof(tmp));
    printf("TrnCode:    [%s]\n", tmp);
    // ȡ Payer
    XmlGetString(doc, "//NoteInfo/Payer", tmp, sizeof(tmp));
    printf("Payer:      [%s]\n", tmp);

    // �޸� Payer
    printf("----------------------\n");
    printf("\n���� Payer = [���Ŵ�]...\n");
    XmlSetString(doc, "//NoteInfo/Payer", "���Ŵ�");

    // DUMP ���ڴ�
    xmlDocDumpMemory(doc, &xmlBuf, &len);
    printf("----------------------\n");
    printf("DUMP���ڴ�(%d):\n[%s]\n", len, xmlBuf);
    xmlFree(xmlBuf);

    // ����doc���ļ�
    if (xmlSaveFile("out.xml", doc) != -1)
        printf("�µ�xml���浽 \"out.xml\".\n");

    xmlFreeDoc(doc);

    // ���libxml�������ú�ռ�õ��ڴ�
    xmlCleanupParser();

    printf("\ntest2 ����!\n");
    return;
}

int test3()
{
    xmlDocPtr doc1, doc2;
    xmlNodePtr node = NULL;

    // ����xml�ļ�
    doc1 = getdoc("dest.xml");
    doc2 = getdoc("src.xml");

    // ȡ���ڵ�ָ��
    node = xmlDocGetRootElement(doc1);

    // �½�һ���ڵ�
    XmlNewTextChild(node, "newchild", "�½�");

    // �滻child�ڵ�
    TestReplaceNode(doc1, doc2, "/root/child");

    node = XmlLocateNodeNew(doc2, "/root/child");
    printf("[%s]\n", xmlNodeGetContent(node));
    // ����xml�ĵ�Ϊ�ļ�
    xmlSaveFile("src_new.xml", doc2);
    xmlSaveFile("dest_new.xml", doc1);

    // �ͷ�docָ��
    xmlFreeDoc(doc1);
    xmlFreeDoc(doc2);

    // ���libxml�������ú�ռ�õ��ڴ�
    xmlCleanupParser();

    printf("\ntest3 ����!\n");
    return 0;
}

#if 0
int main()
{
    char buf[500];
    char tmp[500];
    int len;

    printf("���س���ʼִ�� test1()...");
    getchar();
    test1("test1.xml");

    sprintf(buf, "<?xml version=\"1.0\" encoding=\"GB18030\"?>"
            "<UFTP><MsgHdrRq><TrnCode>0001</TrnCode></MsgHdrRq>"
            "<NoteInfo><Payer>�½�</Payer></NoteInfo></UFTP>");
    printf("���س���ʼִ�� test2()...");
    getchar();
    test2(buf);

    printf("���س���ʼִ�� test3()...");
    getchar();
    test3(buf);

    /*
    strcpy(buf, "��aa");
    len = Iconv(buf, strlen(buf), "GB18030", tmp, sizeof(tmp), "UTF-8");

    printf("[%d][%s]\n", len, buf);
    printf("[%d][%s]\n", len, tmp);
    */
    return 0;
}
#endif

int notetype_c2b(char *notetype, char *bktype, char *bkcode)
{
    xmlDocPtr doc;
    xmlNodePtr node;
    char path[256];
    char *p;

    *bktype = *bkcode = 0x00;
    sprintf(path, "%s/etc/map.xml", getenv("APP_DIR"));
    if ((doc = xmlParseFile(path)) == NULL)
        return -1;

    sprintf(path, "//notetype_c2b/mapping[@name='%s']", notetype);
    if ((node = XmlLocateNode(doc, path)) == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }
    if ((p = XmlNodeGetAttrText(node, "value")) == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }
    strcpy(bktype, p);
    free(p); p = NULL;

    if ((p = XmlNodeGetAttrText(node, "bkcode")) == NULL)
    {
        xmlFreeDoc(doc);
        return -1;
    }
    strcpy(bkcode, p);

    free(p);
    xmlFreeDoc(doc);
    return 0;
}
int main()
{
    xmlDocPtr doc, doc1, doc2;
    xmlNodePtr node = NULL, cur, scur;
    char *grid;
    char path[1024], buf1[100], buf2[100];
    int rc;

    // ����xml�ļ�
    doc = getdoc("../etc/cvt.xml");

    sprintf(path, "/Root/Transaction[@code='sz37']/Response");
    if ((node = XmlLocateNode(doc, path)) == NULL)
    {
        err_log("���ҽڵ�[%s]ʧ��!", path);
        xmlFreeDoc(doc);
        return -1;
    }

    for( cur = node->children; cur != NULL; cur = cur->next )
    {
        if (cur->type != XML_ELEMENT_NODE)
            continue;
        if (!strcmp(cur->name, "grid"))
        {
            for (scur = cur->children; scur != NULL; scur = scur->next)
            {
                if (scur->type != XML_ELEMENT_NODE)
                    continue;
                printf("anode=[%s]\n", scur->name);
            }
        }
        else
            printf("node=[%s]\n", cur->name);
    }

    // �ͷ�docָ��
    xmlFreeDoc(doc);

    rc = notetype_c2b("09", buf1, buf2);
    printf("[%d][%s][%s]\n", rc, buf1, buf2);
    // ���libxml�������ú�ռ�õ��ڴ�
    xmlCleanupParser();

    return 0;
}
