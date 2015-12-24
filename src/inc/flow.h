#ifndef FLOW_H_
#define FLOW_H_

#define FLOWFUNC_PLACE_REGION   'R'         //函数存放在地区动态库中
#define FLOWFUNC_PLACE_BANK     'B'         //函数存放在银行动态库中

typedef struct FlowNode {
    char nodename[24+1];
    char nodedesc[48+1];
    char funcname[24+1];
    char funcplace[1];
} FLowNode;

typedef struct OPFlows {
    FLowNode afterRequest;
    FLowNode afterResponse;
} OPFlows;

extern OPFlows OP_FLOW;

int initFlow();

int callFlow(const char *nodename);

#endif
