#include "app.h"
#include "tcpapi.h"

int main(int argc, char *argv[])
{
    xmlDoc *rspDoc = NULL;
    xmlDoc *reqdoc = NULL;
    unsigned char *req = NULL;
    int len = 0;
    int *plen;
    int result = 0;
    char xpath[256]={0};
    xmlNodePtr tNode = NULL;
    char reqbuf[1024]={0};


    sprintf(reqbuf, "%s\n%s", "<?xml version=\"1.0\" encoding=\"GB18030\"?>",
            "<UFTP><MsgHdrRq><RefId>20642</RefId><TrnCode>0002</TrnCode><SvcClass>1</SvcClass><Recver>1015</Recver><Originator>094300</Originator><Acceptor>024000</Acceptor><WorkDate></WorkDate><AcctOper>11024875</AcctOper><TermId>8907</TermId></MsgHdrRq><NoteInfo><DCFlag>2</DCFlag><NoteType>01</NoteType><NoteNo>60152811</NoteNo><IssueDate>20110701</IssueDate><PayingAcct>9024135025863</PayingAcct><Payer>�ŵ¾��ܹ�ҵ(��ɽ)���޹�˾</Payer><PayingBank></PayingBank><BeneAcct>1102023509005264203</BeneAcct><BeneName>��ɽ�˱��˾���ģ�����޹�˾</BeneName><BeneBank></BeneBank><CurCode>CNY</CurCode><CurType>0</CurType><SettlAmt>7975.00</SettlAmt><IssueAmt>0.00</IssueAmt><RemnAmt>0.00</RemnAmt><PayKey></PayKey><TestKey></TestKey><Agreement></Agreement><Purpose></Purpose><ExtraData><OppBank></OppBank><OppBankName></OppBankName><TrnDetail>get_trndetail���ɵ���ϸ</TrnDetail></ExtraData><TruncFlag></TruncFlag></NoteInfo><SettlInfo><ExchgRound>1</ExchgRound><ExchgDate>20110704</ExchgDate></SettlInfo><BankData><PDWSNO></PDWSNO><QTJYMA>sz37</QTJYMA></BankData></UFTP>" );

    /*
    sprintf(reqbuf, "%s\n%s", "<?xml version=\"1.0\" encoding=\"GB18030\"?>",
            "<UFTP><MsgHdrRq><RefId>20642</RefId><TrnCode>0002</TrnCode><SvcClass>1</SvcClass><Recver>1015</Recver><Originator>094300</Originator><Acceptor>024000</Acceptor><WorkDate></WorkDate><AcctOper>11024875</AcctOper><TermId>8907</TermId></MsgHdrRq><NoteInfo><DCFlag>2</DCFlag><NoteType>01</NoteType><NoteNo>60152811</NoteNo><IssueDate>20110701</IssueDate><PayingAcct>9024135025863</PayingAcct><Payer>�ŵ¾��ܹ�ҵ(��ɽ)���޹�˾</Payer><PayingBank></PayingBank><BeneAcct>1102023509005264203</BeneAcct><BeneName>��ɽ�˱��˾���ģ�����޹�˾</BeneName><BeneBank></BeneBank><CurCode>CNY</CurCode><CurType>0</CurType><SettlAmt>7975.00</SettlAmt><IssueAmt>0.00</IssueAmt><RemnAmt>0.00</RemnAmt><PayKey></PayKey><TestKey></TestKey><Agreement></Agreement><Purpose></Purpose><ExtraData><OppBank></OppBank><OppBankName></OppBankName><TrnDetail>get_trndetail���ɵ���ϸ</TrnDetail></ExtraData><TruncFlag></TruncFlag></NoteInfo><SettlInfo><ExchgRound>1</ExchgRound><ExchgDate>20110705</ExchgDate></SettlInfo></UFTP>" );
            */
    req = reqbuf;
    if ((reqdoc = xmlRecoverDoc(req)) == NULL)
    {
        fprintf(stderr, "����xml�ļ�ʧ��");
    }
    fprintf(stderr, "����xml�ļ��ɹ�\n");

    /*
       sprintf(xpath, "%s/test/test.xml", getenv("HOME"));
       if ((reqdoc = xmlParseFile(xpath)) == NULL)
       fprintf( stderr, "��ʼ���ļ�[%s]ʧ��", xpath);
     */

    sprintf( xpath, "/UFTP/BankData" );
    if ((tNode = sdpXmlSelectNode(reqdoc, xpath)) == NULL) {
        fprintf( stderr, "δ�ҵ��ڵ�[%s]\n", xpath);
    }
    else
    {
        fprintf( stderr, "�ͷŽڵ�[%s]\n", xpath);
        xmlUnlinkNode(tNode);
        req = NULL;
        len = 0;
    }
    //xmlDocDumpMemory(reqdoc, &req, &len);
    xmlDocDumpMemory(reqdoc, &req, &len);
    plen = &len;
    fprintf( stderr, "REQ[%s]\nLen[%d]\n", req, *plen);

    return 0;
}
