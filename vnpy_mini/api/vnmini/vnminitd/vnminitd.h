//ϵͳ
#ifdef WIN32
#include "stdafx.h"
#endif

#include "vnmini.h"
#include "pybind11/pybind11.h"
#include "mini/ThostFtdcTraderApi.h"


using namespace pybind11;

//����
#define ONFRONTCONNECTED 0
#define ONFRONTDISCONNECTED 1
#define ONHEARTBEATWARNING 2
#define ONRSPSUBSCRIBEFLOWCTRLWARNING 3
#define ONRSPUNSUBSCRIBEFLOWCTRLWARNING 4
#define ONRSPAUTHENTICATE 5
#define ONRSPUSERLOGIN 6
#define ONRSPUSERLOGOUT 7
#define ONRSPORDERINSERT 8
#define ONRSPORDERACTION 9
#define ONRSPMKBATCHORDERACTION 10
#define ONRSPEXECORDERINSERT 11
#define ONRSPEXECORDERACTION 12
#define ONRSPFORQUOTEINSERT 13
#define ONRSPQUOTEINSERT 14
#define ONRSPQUOTEACTION 15
#define ONRSPBATCHORDERACTION 16
#define ONRSPOPTIONSELFCLOSEINSERT 17
#define ONRSPOPTIONSELFCLOSEACTION 18
#define ONRSPCOMBACTIONINSERT 19
#define ONRSPQRYORDER 20
#define ONRSPQRYTRADE 21
#define ONRSPQRYINVESTORPOSITION 22
#define ONRSPQRYTRADINGACCOUNT 23
#define ONRSPQRYINVESTOR 24
#define ONRSPQRYTRADINGCODE 25
#define ONRSPQRYINSTRUMENTMARGINRATE 26
#define ONRSPQRYINSTRUMENTCOMMISSIONRATE 27
#define ONRSPQRYEXCHANGE 28
#define ONRSPQRYPRODUCT 29
#define ONRSPQRYINSTRUMENT 30
#define ONRSPQRYCOMBINSTRUMENT 31
#define ONRSPQRYCOMBACTION 32
#define ONRSPQRYINVESTORPOSITIONFORCOMB 33
#define ONRSPQRYDEPTHMARKETDATA 34
#define ONRSPQRYINSTRUMENTSTATUS 35
#define ONRSPQRYINVESTORPOSITIONDETAIL 36
#define ONRSPQRYEXCHANGEMARGINRATE 37
#define ONRSPQRYEXCHANGEMARGINRATEADJUST 38
#define ONRSPQRYOPTIONINSTRTRADECOST 39
#define ONRSPQRYOPTIONINSTRCOMMRATE 40
#define ONRSPQRYEXECORDER 41
#define ONRSPQRYFORQUOTE 42
#define ONRSPQRYFORQUOTEPARAM 43
#define ONRSPQRYINVESTORPRODSPBMDETAIL 44
#define ONRSPQRYTRADEROFFER 45
#define ONRSPQRYQUOTE 46
#define ONRSPQRYOPTIONSELFCLOSE 47
#define ONRSPERROR 48
#define ONRTNORDER 49
#define ONRTNTRADE 50
#define ONERRRTNORDERINSERT 51
#define ONERRRTNORDERACTION 52
#define ONRTNINSTRUMENTSTATUS 53
#define ONRTNEXECORDER 54
#define ONERRRTNEXECORDERINSERT 55
#define ONERRRTNEXECORDERACTION 56
#define ONERRRTNFORQUOTEINSERT 57
#define ONRTNQUOTE 58
#define ONERRRTNQUOTEINSERT 59
#define ONERRRTNQUOTEACTION 60
#define ONRTNFORQUOTERSP 61
#define ONERRRTNBATCHORDERACTION 62
#define ONRTNOPTIONSELFCLOSE 63
#define ONERRRTNOPTIONSELFCLOSEINSERT 64
#define ONERRRTNOPTIONSELFCLOSEACTION 65
#define ONRTNCOMBACTION 66
#define ONRSPQRYINSTRUMENTORDERCOMMRATE 67
#define ONRTNFLOWCTRLWARNING 68

///-------------------------------------------------------------------------------------
///C++ SPI�Ļص���������ʵ��
///-------------------------------------------------------------------------------------

//API�ļ̳�ʵ��
class TdApi : public CThostFtdcTraderSpi
{
private:
	CThostFtdcTraderApi* api;            //API����
    thread task_thread;                    //�����߳�ָ�루��python���������ݣ�
    TaskQueue task_queue;                //�������
    bool active = false;                //����״̬

public:
    TdApi()
    {
    };

    ~TdApi()
    {
        if (this->active)
        {
            this->exit();
        }
    };

    //-------------------------------------------------------------------------------------
    //API�ص�����
    //-------------------------------------------------------------------------------------

	///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
	virtual void OnFrontConnected();

	///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
	///@param nReason ����ԭ��
	///        -3	�ر�����
	///        -4	�����ʧ��
	///        -5	����дʧ��
	///        -6	��������ˮ�������
	///        -7	���кŴ���
	///        -8	����������
	///        -9	������������С
	virtual void OnFrontDisconnected(int nReason);

	///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
	///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
	virtual void OnHeartBeatWarning(int nTimeLapse);

	///�������ؾ���Ӧ��
	virtual void OnRspSubscribeFlowCtrlWarning(CThostFtdcSpecificTraderField* pRspSubscribeTraderField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///ȡ���������ؾ���Ӧ��
	virtual void OnRspUnSubscribeFlowCtrlWarning(CThostFtdcSpecificTraderField* pRspSubscribeTraderField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�ͻ�����֤��Ӧ
	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);


	///��¼������Ӧ
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�ǳ�������Ӧ
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///����¼��������Ӧ
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///��������������Ӧ
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField* pInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///������������������������Ӧ
	virtual void OnRspMKBatchOrderAction(CThostFtdcMKInputOrderActionField* pMKInputOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///ִ������¼��������Ӧ
	virtual void OnRspExecOrderInsert(CThostFtdcInputExecOrderField* pInputExecOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///ִ���������������Ӧ
	virtual void OnRspExecOrderAction(CThostFtdcInputExecOrderActionField* pInputExecOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///ѯ��¼��������Ӧ
	virtual void OnRspForQuoteInsert(CThostFtdcInputForQuoteField* pInputForQuote, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///����¼��������Ӧ
	virtual void OnRspQuoteInsert(CThostFtdcInputQuoteField* pInputQuote, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///���۲���������Ӧ
	virtual void OnRspQuoteAction(CThostFtdcInputQuoteActionField* pInputQuoteAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///������������������Ӧ
	virtual void OnRspBatchOrderAction(CThostFtdcInputBatchOrderActionField* pInputBatchOrderAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///��Ȩ�ԶԳ�¼��������Ӧ
	virtual void OnRspOptionSelfCloseInsert(CThostFtdcInputOptionSelfCloseField* pInputOptionSelfClose, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///��Ȩ�ԶԳ����������Ӧ
	virtual void OnRspOptionSelfCloseAction(CThostFtdcInputOptionSelfCloseActionField* pInputOptionSelfCloseAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�������¼��������Ӧ
	virtual void OnRspCombActionInsert(CThostFtdcInputCombActionField* pInputCombAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ������Ӧ
	virtual void OnRspQryOrder(CThostFtdcOrderField* pOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ�ɽ���Ӧ
	virtual void OnRspQryTrade(CThostFtdcTradeField* pTrade, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯͶ���ֲ߳���Ӧ
	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField* pInvestorPosition, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ�ʽ��˻���Ӧ
	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField* pTradingAccount, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯͶ������Ӧ
	virtual void OnRspQryInvestor(CThostFtdcInvestorField* pInvestor, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ���ױ�����Ӧ
	virtual void OnRspQryTradingCode(CThostFtdcTradingCodeField* pTradingCode, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��Լ��֤������Ӧ
	virtual void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField* pInstrumentMarginRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��Լ����������Ӧ
	virtual void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField* pInstrumentCommissionRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��������Ӧ
	virtual void OnRspQryExchange(CThostFtdcExchangeField* pExchange, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��Ʒ��Ӧ
	virtual void OnRspQryProduct(CThostFtdcProductField* pProduct, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��Լ��Ӧ
	virtual void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ������Ϻ�Լ��Ӧ
	virtual void OnRspQryCombInstrument(CThostFtdcCombInstrumentField* pCombInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ���������Ӧ
	virtual void OnRspQryCombAction(CThostFtdcCombActionField* pCombAction, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��ϵ��Ȼ��ܱ���Ӧ
	virtual void OnRspQryInvestorPositionForComb(CThostFtdcInvestorPositionForCombField* pForComb, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ������Ӧ
	virtual void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��Լ״̬��Ӧ
	virtual void OnRspQryInstrumentStatus(CThostFtdcInstrumentStatusField* pInstrumentStatus, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯͶ���ֲ߳���ϸ��Ӧ
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��������֤������Ӧ
	virtual void OnRspQryExchangeMarginRate(CThostFtdcExchangeMarginRateField* pExchangeMarginRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ������������֤������Ӧ
	virtual void OnRspQryExchangeMarginRateAdjust(CThostFtdcExchangeMarginRateAdjustField* pExchangeMarginRateAdjust, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��Ȩ���׳ɱ���Ӧ
	virtual void OnRspQryOptionInstrTradeCost(CThostFtdcOptionInstrTradeCostField* pOptionInstrTradeCost, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��Ȩ��Լ��������Ӧ
	virtual void OnRspQryOptionInstrCommRate(CThostFtdcOptionInstrCommRateField* pOptionInstrCommRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯִ��������Ӧ
	virtual void OnRspQryExecOrder(CThostFtdcExecOrderField* pExecOrder, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯѯ����Ӧ
	virtual void OnRspQryForQuote(CThostFtdcForQuoteField* pForQuote, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯѯ�ۼ۲���Ӧ
	virtual void OnRspQryForQuoteParam(CThostFtdcForQuoteParamField* pForQuoteParam, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯͶ����SPBMƷ����ϸ��Ӧ
	virtual void OnRspQryInvestorProdSPBMDetail(CThostFtdcInvestorProdSPBMDetailField* pInvestorProdSPBMDetail, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ����Ա���̻���Ӧ
	virtual void OnRspQryTraderOffer(CThostFtdcTraderOfferField* pTraderOffer, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ������Ӧ
	virtual void OnRspQryQuote(CThostFtdcQuoteField* pQuote, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ��Ȩ�ԶԳ���Ӧ
	virtual void OnRspQryOptionSelfClose(CThostFtdcOptionSelfCloseField* pOptionSelfClose, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///����Ӧ��
	virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///����֪ͨ
	virtual void OnRtnOrder(CThostFtdcOrderField* pOrder);

	///�ɽ�֪ͨ
	virtual void OnRtnTrade(CThostFtdcTradeField* pTrade);

	///����¼�����ر�
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo);

	///������������ر�
	virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction, CThostFtdcRspInfoField* pRspInfo);

	///��Լ����״̬֪ͨ
	virtual void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField* pInstrumentStatus);

	///ִ������֪ͨ
	virtual void OnRtnExecOrder(CThostFtdcExecOrderField* pExecOrder);

	///ִ������¼�����ر�
	virtual void OnErrRtnExecOrderInsert(CThostFtdcInputExecOrderField* pInputExecOrder, CThostFtdcRspInfoField* pRspInfo);

	///ִ�������������ر�
	virtual void OnErrRtnExecOrderAction(CThostFtdcExecOrderActionField* pExecOrderAction, CThostFtdcRspInfoField* pRspInfo);

	///ѯ��¼�����ر�
	virtual void OnErrRtnForQuoteInsert(CThostFtdcInputForQuoteField* pInputForQuote, CThostFtdcRspInfoField* pRspInfo);

	///����֪ͨ
	virtual void OnRtnQuote(CThostFtdcQuoteField* pQuote);

	///����¼�����ر�
	virtual void OnErrRtnQuoteInsert(CThostFtdcInputQuoteField* pInputQuote, CThostFtdcRspInfoField* pRspInfo);

	///���۲�������ر�
	virtual void OnErrRtnQuoteAction(CThostFtdcQuoteActionField* pQuoteAction, CThostFtdcRspInfoField* pRspInfo);

	///ѯ��֪ͨ
	virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField* pForQuoteRsp);

	///����������������ر�
	virtual void OnErrRtnBatchOrderAction(CThostFtdcBatchOrderActionField* pBatchOrderAction, CThostFtdcRspInfoField* pRspInfo);

	///��Ȩ�ԶԳ�֪ͨ
	virtual void OnRtnOptionSelfClose(CThostFtdcOptionSelfCloseField* pOptionSelfClose);

	///��Ȩ�ԶԳ�¼�����ر�
	virtual void OnErrRtnOptionSelfCloseInsert(CThostFtdcInputOptionSelfCloseField* pInputOptionSelfClose, CThostFtdcRspInfoField* pRspInfo);

	///��Ȩ�ԶԳ��������ر�
	virtual void OnErrRtnOptionSelfCloseAction(CThostFtdcOptionSelfCloseActionField* pOptionSelfCloseAction, CThostFtdcRspInfoField* pRspInfo);

	///�������֪ͨ
	virtual void OnRtnCombAction(CThostFtdcCombActionField* pCombAction);

	///�����ѯ�걨����Ӧ
	virtual void OnRspQryInstrumentOrderCommRate(CThostFtdcInstrumentOrderCommRateField* pInstrumentOrderCommRate, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast);

	///������ϯλ���ؾ���
	virtual void OnRtnFlowCtrlWarning(CThostFtdcFlowCtrlWarningField* pFlowCtrlWarning);

    //-------------------------------------------------------------------------------------
    //task������
    //-------------------------------------------------------------------------------------
    void processTask();

	void processFrontConnected(Task* task);

	void processFrontDisconnected(Task* task);

	void processHeartBeatWarning(Task* task);

	void processRspSubscribeFlowCtrlWarning(Task* task);

	void processRspUnSubscribeFlowCtrlWarning(Task* task);

	void processRspAuthenticate(Task* task);

	void processRspUserLogin(Task* task);

	void processRspUserLogout(Task* task);

	void processRspOrderInsert(Task* task);

	void processRspOrderAction(Task* task);

	void processRspMKBatchOrderAction(Task* task);

	void processRspExecOrderInsert(Task* task);

	void processRspExecOrderAction(Task* task);

	void processRspForQuoteInsert(Task* task);

	void processRspQuoteInsert(Task* task);

	void processRspQuoteAction(Task* task);

	void processRspBatchOrderAction(Task* task);

	void processRspOptionSelfCloseInsert(Task* task);

	void processRspOptionSelfCloseAction(Task* task);

	void processRspCombActionInsert(Task* task);

	void processRspQryOrder(Task* task);

	void processRspQryTrade(Task* task);

	void processRspQryInvestorPosition(Task* task);

	void processRspQryTradingAccount(Task* task);

	void processRspQryInvestor(Task* task);

	void processRspQryTradingCode(Task* task);

	void processRspQryInstrumentMarginRate(Task* task);

	void processRspQryInstrumentCommissionRate(Task* task);

	void processRspQryExchange(Task* task);

	void processRspQryProduct(Task* task);

	void processRspQryInstrument(Task* task);

	void processRspQryCombInstrument(Task* task);

	void processRspQryCombAction(Task* task);

	void processRspQryInvestorPositionForComb(Task* task);

	void processRspQryDepthMarketData(Task* task);

	void processRspQryInstrumentStatus(Task* task);

	void processRspQryInvestorPositionDetail(Task* task);

	void processRspQryExchangeMarginRate(Task* task);

	void processRspQryExchangeMarginRateAdjust(Task* task);

	void processRspQryOptionInstrTradeCost(Task* task);

	void processRspQryOptionInstrCommRate(Task* task);

	void processRspQryExecOrder(Task* task);

	void processRspQryForQuote(Task* task);

	void processRspQryForQuoteParam(Task* task);

	void processRspQryInvestorProdSPBMDetail(Task* task);

	void processRspQryTraderOffer(Task* task);

	void processRspQryQuote(Task* task);

	void processRspQryOptionSelfClose(Task* task);

	void processRspError(Task* task);

	void processRtnOrder(Task* task);

	void processRtnTrade(Task* task);

	void processErrRtnOrderInsert(Task* task);

	void processErrRtnOrderAction(Task* task);

	void processRtnInstrumentStatus(Task* task);

	void processRtnExecOrder(Task* task);

	void processErrRtnExecOrderInsert(Task* task);

	void processErrRtnExecOrderAction(Task* task);

	void processErrRtnForQuoteInsert(Task* task);

	void processRtnQuote(Task* task);

	void processErrRtnQuoteInsert(Task* task);

	void processErrRtnQuoteAction(Task* task);

	void processRtnForQuoteRsp(Task* task);

	void processErrRtnBatchOrderAction(Task* task);

	void processRtnOptionSelfClose(Task* task);

	void processErrRtnOptionSelfCloseInsert(Task* task);

	void processErrRtnOptionSelfCloseAction(Task* task);

	void processRtnCombAction(Task* task);

	void processRspQryInstrumentOrderCommRate(Task* task);

	void processRtnFlowCtrlWarning(Task* task);

    //-------------------------------------------------------------------------------------
    //data���ص������������ֵ�
    //error���ص������Ĵ����ֵ�
    //id������id
    //last���Ƿ�Ϊ��󷵻�
    //i������
    //-------------------------------------------------------------------------------------
    
	virtual void onFrontConnected() {};

	virtual void onFrontDisconnected(int reqid) {};

	virtual void onHeartBeatWarning(int reqid) {};

	virtual void onRspSubscribeFlowCtrlWarning(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspUnSubscribeFlowCtrlWarning(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspAuthenticate(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspUserLogin(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspUserLogout(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspOrderInsert(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspOrderAction(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspMKBatchOrderAction(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspExecOrderInsert(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspExecOrderAction(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspForQuoteInsert(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQuoteInsert(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQuoteAction(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspBatchOrderAction(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspOptionSelfCloseInsert(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspOptionSelfCloseAction(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspCombActionInsert(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryOrder(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryTrade(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryInvestorPosition(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryTradingAccount(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryInvestor(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryTradingCode(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryInstrumentMarginRate(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryInstrumentCommissionRate(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryExchange(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryProduct(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryInstrument(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryCombInstrument(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryCombAction(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryInvestorPositionForComb(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryDepthMarketData(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryInstrumentStatus(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryInvestorPositionDetail(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryExchangeMarginRate(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryExchangeMarginRateAdjust(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryOptionInstrTradeCost(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryOptionInstrCommRate(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryExecOrder(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryForQuote(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryForQuoteParam(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryInvestorProdSPBMDetail(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryTraderOffer(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryQuote(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspQryOptionSelfClose(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRspError(const dict& error, int reqid, bool last) {};

	virtual void onRtnOrder(const dict& data) {};

	virtual void onRtnTrade(const dict& data) {};

	virtual void onErrRtnOrderInsert(const dict& data, const dict& error) {};

	virtual void onErrRtnOrderAction(const dict& data, const dict& error) {};

	virtual void onRtnInstrumentStatus(const dict& data) {};

	virtual void onRtnExecOrder(const dict& data) {};

	virtual void onErrRtnExecOrderInsert(const dict& data, const dict& error) {};

	virtual void onErrRtnExecOrderAction(const dict& data, const dict& error) {};

	virtual void onErrRtnForQuoteInsert(const dict& data, const dict& error) {};

	virtual void onRtnQuote(const dict& data) {};

	virtual void onErrRtnQuoteInsert(const dict& data, const dict& error) {};

	virtual void onErrRtnQuoteAction(const dict& data, const dict& error) {};

	virtual void onRtnForQuoteRsp(const dict& data) {};

	virtual void onErrRtnBatchOrderAction(const dict& data, const dict& error) {};

	virtual void onRtnOptionSelfClose(const dict& data) {};

	virtual void onErrRtnOptionSelfCloseInsert(const dict& data, const dict& error) {};

	virtual void onErrRtnOptionSelfCloseAction(const dict& data, const dict& error) {};

	virtual void onRtnCombAction(const dict& data) {};

	virtual void onRspQryInstrumentOrderCommRate(const dict& data, const dict& error, int reqid, bool last) {};

	virtual void onRtnFlowCtrlWarning(const dict& data) {};

    //-------------------------------------------------------------------------------------
    //req:���������������ֵ�
    //-------------------------------------------------------------------------------------

    void createFtdcTraderApi(string pszFlowPath = "");

    void release();

    void init();

    int join();

    int exit();

	string getApiVersion();

    string getTradingDay();

    void registerFront(string pszFrontAddress);

    void subscribePrivateTopic(int nType);

    void subscribePublicTopic(int nType);

	int reqAuthenticate(const dict &req, int reqid);

	int reqUserLogin(const dict& req, int reqid);

	int reqUserLoginEncrypt(const dict& req, int reqid);

	int reqUserLogout(const dict& req, int reqid);

	int reqOrderInsert(const dict& req, int reqid);

	int reqOrderAction(const dict& req, int reqid);

	int reqMKBatchOrderAction(const dict& req, int reqid);

	int reqExecOrderInsert(const dict& req, int reqid);

	int reqExecOrderAction(const dict& req, int reqid);

	int reqForQuoteInsert(const dict& req, int reqid);

	int reqQuoteInsert(const dict& req, int reqid);

	int reqQuoteAction(const dict& req, int reqid);

	int reqBatchOrderAction(const dict& req, int reqid);

	int reqOptionSelfCloseInsert(const dict& req, int reqid);

	int reqOptionSelfCloseAction(const dict& req, int reqid);

	int reqCombActionInsert(const dict& req, int reqid);

	int reqQryOrder(const dict& req, int reqid);

	int reqQryTrade(const dict& req, int reqid);

	int reqQryInvestorPosition(const dict& req, int reqid);

	int reqQryTradingAccount(const dict& req, int reqid);

	int reqQryInvestor(const dict& req, int reqid);

	int reqQryTradingCode(const dict& req, int reqid);

	int reqQryInstrumentMarginRate(const dict& req, int reqid);

	int reqQryInstrumentCommissionRate(const dict& req, int reqid);

	int reqQryExchange(const dict& req, int reqid);

	int reqQryProduct(const dict& req, int reqid);

	int reqQryInstrument(const dict& req, int reqid);

	int reqQryCombInstrument(const dict& req, int reqid);

	int reqQryInvestorPositionForComb(const dict& req, int reqid);

	int reqQryCombAction(const dict& req, int reqid);

	int reqQryDepthMarketData(const dict& req, int reqid);

	int reqQryOptionSelfClose(const dict& req, int reqid);

	int reqQryInstrumentStatus(const dict& req, int reqid);

	int reqQryInvestorPositionDetail(const dict& req, int reqid);

	int reqQryExchangeMarginRate(const dict& req, int reqid);

	int reqQryExchangeMarginRateAdjust(const dict& req, int reqid);

	int reqQryOptionInstrTradeCost(const dict& req, int reqid);

	int reqQryOptionInstrCommRate(const dict& req, int reqid);

	int reqQryExecOrder(const dict& req, int reqid);

	int reqQryForQuote(const dict& req, int reqid);

	int reqQryQuote(const dict& req, int reqid);

	int reqQryInstrumentOrderCommRate(const dict& req, int reqid);

	int reqQryForQuoteParam(const dict& req, int reqid);

	int reqQryTraderOffer(const dict& req, int reqid);

	int reqQryInvestorProdSPBMDetail(const dict& req, int reqid);
};
