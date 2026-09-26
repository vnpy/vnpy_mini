from collections.abc import Callable, Iterator
from datetime import datetime
from typing import Any

import pytest

pytest.importorskip("vnpy_mini.api", reason="缺少 Mini 原生扩展")

from vnpy.event import Event, EventEngine  # noqa: E402
from vnpy.trader.constant import (  # noqa: E402
    Direction,
    Exchange,
    Offset,
    OptionType,
    OrderType,
    Product,
    Status,
)
from vnpy.trader.event import EVENT_TIMER  # noqa: E402
from vnpy.trader.object import (  # noqa: E402
    AccountData,
    CancelRequest,
    ContractData,
    OrderData,
    OrderRequest,
    PositionData,
    SubscribeRequest,
    TickData,
    TradeData,
)

from vnpy_mini.api import (  # noqa: E402
    THOST_FTDC_CP_CallOptions,
    THOST_FTDC_D_Buy,
    THOST_FTDC_OFEN_Close,
    THOST_FTDC_OF_Open,
    THOST_FTDC_OPT_LimitPrice,
    THOST_FTDC_OST_Canceled,
    THOST_FTDC_OST_NoTradeQueueing,
    THOST_FTDC_PC_Futures,
    THOST_FTDC_PC_Options,
    THOST_FTDC_PD_Long,
    THOST_FTDC_TC_GFD,
    THOST_FTDC_VC_AV,
)
from vnpy_mini.gateway import mini_gateway  # noqa: E402
from vnpy_mini.gateway.mini_gateway import (  # noqa: E402
    CHINA_TZ,
    MAX_FLOAT,
    MiniGateway,
    MiniMdApi,
    MiniTdApi,
    adjust_price,
)


class Sink:
    def __init__(self) -> None:
        self.logs: list[str] = []
        self.ticks: list[TickData] = []
        self.contracts: list[ContractData] = []
        self.orders: list[OrderData] = []
        self.trades: list[TradeData] = []
        self.positions: list[PositionData] = []
        self.accounts: list[AccountData] = []

    def attach(self, gateway: MiniGateway) -> None:
        gateway.write_log = self.logs.append
        gateway.on_tick = self.ticks.append
        gateway.on_contract = self.contracts.append
        gateway.on_order = self.orders.append
        gateway.on_trade = self.trades.append
        gateway.on_position = self.positions.append
        gateway.on_account = self.accounts.append


class CallRecorder:
    def __init__(self) -> None:
        self.calls: list[tuple[str, Any]] = []

    def patch(self, monkeypatch: pytest.MonkeyPatch, api: object, names: list[str]) -> None:
        for name in names:
            monkeypatch.setattr(api, name, self.make_stub(name))

    def make_stub(self, name: str) -> Callable[..., int]:
        def stub(*args: Any) -> int:
            self.calls.append((name, args[0] if args else None))
            return 0
        return stub

    def names(self) -> list[str]:
        return [name for name, _ in self.calls]


TD_METHODS: list[str] = [
    "createFtdcTraderApi",
    "subscribePrivateTopic",
    "subscribePublicTopic",
    "registerFront",
    "init",
    "exit",
    "reqAuthenticate",
    "reqUserLogin",
    "reqQryInstrument",
    "reqOrderInsert",
    "reqOrderAction",
    "reqQryTradingAccount",
    "reqQryInvestorPosition",
]

MD_METHODS: list[str] = [
    "createFtdcMdApi",
    "registerFront",
    "init",
    "exit",
    "reqUserLogin",
    "subscribeMarketData",
]


@pytest.fixture(autouse=True)
def clear_contracts() -> Iterator[None]:
    mini_gateway.symbol_contract_map.clear()
    yield
    mini_gateway.symbol_contract_map.clear()


@pytest.fixture
def sink() -> Sink:
    return Sink()


@pytest.fixture
def recorder() -> CallRecorder:
    return CallRecorder()


@pytest.fixture
def gateway(sink: Sink, recorder: CallRecorder, monkeypatch: pytest.MonkeyPatch) -> MiniGateway:
    engine: EventEngine = EventEngine()
    gateway: MiniGateway = MiniGateway(engine, "MINI")
    sink.attach(gateway)
    recorder.patch(monkeypatch, gateway.td_api, TD_METHODS)
    recorder.patch(monkeypatch, gateway.md_api, MD_METHODS)
    return gateway


@pytest.fixture
def td_api(gateway: MiniGateway) -> MiniTdApi:
    return gateway.td_api


@pytest.fixture
def md_api(gateway: MiniGateway) -> MiniMdApi:
    return gateway.md_api


def add_contract(
    symbol: str = "rb2510",
    exchange: Exchange = Exchange.SHFE,
    size: int = 10,
) -> ContractData:
    contract: ContractData = ContractData(
        symbol=symbol,
        exchange=exchange,
        name=symbol,
        product=Product.FUTURES,
        size=size,
        pricetick=1,
        gateway_name="MINI",
    )
    mini_gateway.symbol_contract_map[symbol] = contract
    return contract


def order_request(
    symbol: str = "rb2510",
    order_type: OrderType = OrderType.LIMIT,
    offset: Offset = Offset.OPEN,
) -> OrderRequest:
    return OrderRequest(
        symbol=symbol,
        exchange=Exchange.SHFE,
        direction=Direction.LONG,
        type=order_type,
        volume=2,
        price=3000,
        offset=offset,
    )


def instrument_data(**overrides: Any) -> dict[str, Any]:
    data: dict[str, Any] = {
        "ProductClass": THOST_FTDC_PC_Futures,
        "InstrumentID": "rb2510",
        "ExchangeID": "SHFE",
        "InstrumentName": "螺纹钢",
        "VolumeMultiple": 10,
        "PriceTick": 1,
    }
    data.update(overrides)
    return data


def depth_data(**overrides: Any) -> dict[str, Any]:
    data: dict[str, Any] = {
        "InstrumentID": "rb2510",
        "UpdateTime": "09:30:00",
        "UpdateMillisec": 500,
        "ActionDay": "20200101",
        "Volume": 10,
        "Turnover": 30000,
        "OpenInterest": 100,
        "LastPrice": 3000,
        "UpperLimitPrice": 3300,
        "LowerLimitPrice": 2700,
        "OpenPrice": 2990,
        "HighestPrice": 3010,
        "LowestPrice": 2980,
        "PreClosePrice": 2985,
        "BidPrice1": 2999,
        "AskPrice1": 3001,
        "BidVolume1": 5,
        "AskVolume1": 6,
        "BidPrice2": 2998,
        "BidPrice3": 2997,
        "BidPrice4": 2996,
        "BidPrice5": 2995,
        "AskPrice2": 3002,
        "AskPrice3": 3003,
        "AskPrice4": 3004,
        "AskPrice5": 3005,
        "BidVolume2": 0,
        "BidVolume3": 0,
        "BidVolume4": 0,
        "BidVolume5": 0,
        "AskVolume2": 0,
        "AskVolume3": 0,
        "AskVolume4": 0,
        "AskVolume5": 0,
    }
    data.update(overrides)
    return data


def rtn_order(**overrides: Any) -> dict[str, Any]:
    data: dict[str, Any] = {
        "InstrumentID": "rb2510",
        "InsertTime": "09:30:00",
        "OrderStatus": THOST_FTDC_OST_NoTradeQueueing,
        "FrontID": 1,
        "SessionID": 2,
        "OrderRef": "7",
        "OrderPriceType": THOST_FTDC_OPT_LimitPrice,
        "TimeCondition": THOST_FTDC_TC_GFD,
        "VolumeCondition": THOST_FTDC_VC_AV,
        "Direction": THOST_FTDC_D_Buy,
        "CombOffsetFlag": THOST_FTDC_OF_Open,
        "LimitPrice": 3000,
        "VolumeTotalOriginal": 2,
        "VolumeTraded": 0,
        "OrderSysID": "SYS1",
    }
    data.update(overrides)
    return data


def position_data(**overrides: Any) -> dict[str, Any]:
    data: dict[str, Any] = {
        "InstrumentID": "rb2510",
        "PosiDirection": THOST_FTDC_PD_Long,
        "YdPosition": 1,
        "TodayPosition": 0,
        "Position": 2,
        "PositionProfit": 12.5,
        "PositionCost": 60000,
        "ShortFrozen": 1,
        "LongFrozen": 4,
    }
    data.update(overrides)
    return data


def test_adjust_price_replaces_max_float() -> None:
    assert adjust_price(MAX_FLOAT) == 0
    assert adjust_price(3000) == 3000


def test_connect_prefixes_bare_address(gateway: MiniGateway, monkeypatch: pytest.MonkeyPatch) -> None:
    seen: dict[str, str] = {}
    monkeypatch.setattr(gateway.td_api, "connect", lambda *args: seen.__setitem__("td", args[0]))
    monkeypatch.setattr(gateway.md_api, "connect", lambda *args: seen.__setitem__("md", args[0]))

    setting: dict[str, str] = dict(MiniGateway.default_setting)
    setting["交易服务器"] = "127.0.0.1:41205"
    setting["行情服务器"] = "ssl://127.0.0.1:41213"
    gateway.connect(setting)

    assert seen["td"] == "tcp://127.0.0.1:41205"
    assert seen["md"] == "ssl://127.0.0.1:41213"


def test_connect_keeps_socks_prefix(gateway: MiniGateway, monkeypatch: pytest.MonkeyPatch) -> None:
    seen: dict[str, str] = {}
    monkeypatch.setattr(gateway.td_api, "connect", lambda *args: seen.__setitem__("td", args[0]))
    monkeypatch.setattr(gateway.md_api, "connect", lambda *args: seen.__setitem__("md", args[0]))

    setting: dict[str, str] = dict(MiniGateway.default_setting)
    setting["交易服务器"] = "socks://127.0.0.1:1080"
    setting["行情服务器"] = "socks://127.0.0.1:1080"
    gateway.connect(setting)

    assert seen["td"] == "socks://127.0.0.1:1080"
    assert seen["md"] == "socks://127.0.0.1:1080"


def test_timer_skips_first_tick_then_rotates_queries(gateway: MiniGateway) -> None:
    calls: list[str] = []
    gateway.query_account = lambda: calls.append("account")  # type: ignore[method-assign]
    gateway.query_position = lambda: calls.append("position")  # type: ignore[method-assign]
    gateway.init_query()
    event: Event = Event(EVENT_TIMER)

    gateway.process_timer_event(event)
    assert calls == []

    gateway.process_timer_event(event)
    gateway.process_timer_event(event)
    gateway.process_timer_event(event)

    assert calls == ["account", "position"]


def test_front_connected_without_auth_logs_in(td_api: MiniTdApi, recorder: CallRecorder) -> None:
    td_api.auth_code = ""
    td_api.onFrontConnected()

    assert recorder.names() == ["reqUserLogin"]


def test_front_connected_with_auth_authenticates(td_api: MiniTdApi, recorder: CallRecorder) -> None:
    td_api.auth_code = "AUTH"
    td_api.appid = "app"
    td_api.onFrontConnected()

    assert recorder.names() == ["reqAuthenticate"]
    assert recorder.calls[0][1]["AuthCode"] == "AUTH"


def test_login_failed_does_not_send_again(td_api: MiniTdApi, recorder: CallRecorder) -> None:
    td_api.login_failed = True
    td_api.login()

    assert recorder.calls == []


def test_send_order_requires_offset(td_api: MiniTdApi, sink: Sink, recorder: CallRecorder) -> None:
    assert td_api.send_order(order_request(offset=Offset.NONE)) == ""
    assert recorder.names() == []
    assert sink.logs == ["请选择开平方向"]


def test_send_order_rejects_stop(td_api: MiniTdApi, sink: Sink) -> None:
    assert td_api.send_order(order_request(order_type=OrderType.STOP)) == ""
    assert sink.orders == []


def test_send_order_returns_local_id(td_api: MiniTdApi, sink: Sink, recorder: CallRecorder) -> None:
    td_api.frontid = 1
    td_api.sessionid = 2
    td_api.userid = "u1"
    td_api.brokerid = "9999"

    vt_orderid: str = td_api.send_order(order_request())

    assert vt_orderid == "MINI.1_2_1"
    request: dict[str, Any] = recorder.calls[0][1]
    assert request["OrderPriceType"] == THOST_FTDC_OPT_LimitPrice
    assert request["TimeCondition"] == THOST_FTDC_TC_GFD
    assert request["VolumeCondition"] == THOST_FTDC_VC_AV
    assert request["CombOffsetFlag"] == THOST_FTDC_OF_Open
    assert request["VolumeTotalOriginal"] == 2
    assert sink.orders[0].status == Status.SUBMITTING


def test_send_order_insert_error_returns_empty(
    td_api: MiniTdApi,
    sink: Sink,
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    monkeypatch.setattr(td_api, "reqOrderInsert", lambda req, reqid: -1)

    assert td_api.send_order(order_request()) == ""
    assert sink.orders == []
    assert "委托请求发送失败" in sink.logs[0]


def test_cancel_order_splits_local_id(td_api: MiniTdApi, recorder: CallRecorder) -> None:
    td_api.userid = "u1"
    td_api.brokerid = "9999"
    td_api.cancel_order(CancelRequest(orderid="1_2_7", symbol="rb2510", exchange=Exchange.SHFE))

    request: dict[str, Any] = recorder.calls[0][1]
    assert request["FrontID"] == 1
    assert request["SessionID"] == 2
    assert request["OrderRef"] == "7"
    assert request["InstrumentID"] == "rb2510"


def test_query_position_waits_for_contracts(td_api: MiniTdApi, recorder: CallRecorder) -> None:
    td_api.query_position()

    assert recorder.calls == []


def test_query_position_sends_investor(td_api: MiniTdApi, recorder: CallRecorder) -> None:
    add_contract()
    td_api.userid = "u1"
    td_api.brokerid = "9999"
    td_api.query_position()

    assert recorder.calls == [("reqQryInvestorPosition", {"BrokerID": "9999", "InvestorID": "u1"})]


def test_instrument_futures_cached(td_api: MiniTdApi, sink: Sink) -> None:
    td_api.onRspQryInstrument(instrument_data(ExchangeID="GFEX", InstrumentID="si2501"), {}, 1, True)

    contract: ContractData = sink.contracts[0]
    assert contract.symbol == "si2501"
    assert contract.exchange == Exchange.GFEX
    assert contract.product == Product.FUTURES
    assert td_api.contract_inited is True
    assert mini_gateway.symbol_contract_map["si2501"] is contract


def test_czce_option_portfolio_drops_suffix(td_api: MiniTdApi, sink: Sink) -> None:
    data: dict[str, Any] = instrument_data(
        ProductClass=THOST_FTDC_PC_Options,
        InstrumentID="SR501C5000",
        ExchangeID="CZCE",
        ProductID="SR501C",
        UnderlyingInstrID="SR501",
        OptionsType=THOST_FTDC_CP_CallOptions,
        StrikePrice=5000,
        OpenDate="20250101",
        ExpireDate="20250115",
    )
    td_api.onRspQryInstrument(data, {}, 1, False)

    contract: ContractData = sink.contracts[0]
    assert contract.option_portfolio == "SR501"
    assert contract.option_underlying == "SR501"
    assert contract.option_type == OptionType.CALL
    assert contract.option_strike == 5000
    assert contract.option_expiry == datetime(2025, 1, 15)


def test_shfe_option_portfolio_keeps_product_id(td_api: MiniTdApi, sink: Sink) -> None:
    data: dict[str, Any] = instrument_data(
        ProductClass=THOST_FTDC_PC_Options,
        InstrumentID="rb2510C3000",
        ExchangeID="SHFE",
        ProductID="rb_o",
        UnderlyingInstrID="rb2510",
        OptionsType=THOST_FTDC_CP_CallOptions,
        StrikePrice=3000,
        OpenDate="20250101",
        ExpireDate="20250915",
    )
    td_api.onRspQryInstrument(data, {}, 1, False)

    assert sink.contracts[0].option_portfolio == "rb_o"


def test_order_before_contract_is_replayed(td_api: MiniTdApi, sink: Sink) -> None:
    add_contract()
    td_api.trading_date = "20250926"
    td_api.onRtnOrder(rtn_order())
    assert sink.orders == []

    td_api.onRspQryInstrument(instrument_data(), {}, 1, True)

    order: OrderData = sink.orders[0]
    assert order.orderid == "1_2_7"
    assert order.status == Status.NOTTRADED
    assert order.datetime == datetime(2025, 9, 26, 9, 30, tzinfo=CHINA_TZ)
    assert td_api.order_data == []
    assert td_api.sysid_orderid_map["SYS1"] == "1_2_7"


def test_unsupported_order_type_is_logged(td_api: MiniTdApi, sink: Sink) -> None:
    add_contract()
    td_api.contract_inited = True
    td_api.trading_date = "20250926"
    td_api.onRtnOrder(rtn_order(OrderPriceType="Z"))

    assert sink.orders == []
    assert "不支持的委托类型" in sink.logs[0]


def test_cancelled_order_without_insert_time_uses_trading_date(td_api: MiniTdApi, sink: Sink) -> None:
    add_contract()
    td_api.contract_inited = True
    td_api.trading_date = "20250926"
    td_api.onRtnOrder(rtn_order(InsertTime="", OrderStatus=THOST_FTDC_OST_Canceled))

    order: OrderData = sink.orders[0]
    assert order.status == Status.CANCELLED
    assert order.datetime is not None
    assert order.datetime.strftime("%Y%m%d") == "20250926"
    assert order.datetime.tzinfo == CHINA_TZ


def test_order_without_time_and_not_cancelled_is_ignored(td_api: MiniTdApi, sink: Sink) -> None:
    add_contract()
    td_api.contract_inited = True
    td_api.trading_date = "20250926"
    td_api.onRtnOrder(rtn_order(InsertTime=""))

    assert sink.orders == []


def test_trade_uses_sysid_map(td_api: MiniTdApi, sink: Sink) -> None:
    add_contract()
    td_api.contract_inited = True
    td_api.sysid_orderid_map["SYS1"] = "1_2_7"
    td_api.onRtnTrade({
        "InstrumentID": "rb2510",
        "OrderSysID": "SYS1",
        "TradeID": "T1",
        "Direction": THOST_FTDC_D_Buy,
        "OffsetFlag": THOST_FTDC_OFEN_Close,
        "Price": 3001,
        "Volume": 1,
        "TradeDate": "20250926",
        "TradeTime": "09:31:00",
    })

    trade: TradeData = sink.trades[0]
    assert trade.orderid == "1_2_7"
    assert trade.tradeid == "T1"
    assert trade.offset == Offset.CLOSE
    assert trade.datetime == datetime(2025, 9, 26, 9, 31, tzinfo=CHINA_TZ)


def test_trade_before_contract_is_replayed(td_api: MiniTdApi, sink: Sink) -> None:
    add_contract()
    td_api.sysid_orderid_map["SYS1"] = "1_2_7"
    td_api.onRtnTrade({
        "InstrumentID": "rb2510",
        "OrderSysID": "SYS1",
        "TradeID": "T1",
        "Direction": THOST_FTDC_D_Buy,
        "OffsetFlag": THOST_FTDC_OF_Open,
        "Price": 3001,
        "Volume": 1,
        "TradeDate": "20250926",
        "TradeTime": "09:31:00",
    })
    assert sink.trades == []

    td_api.onRspQryInstrument(instrument_data(), {}, 1, True)

    assert sink.trades[0].tradeid == "T1"


def test_shfe_yd_position_uses_position_when_only_yesterday(td_api: MiniTdApi, sink: Sink) -> None:
    add_contract(exchange=Exchange.SHFE, size=10)
    td_api.onRspQryInvestorPosition(position_data(), {}, 1, True)
    assert sink.positions == []

    td_api.onRspQryInvestorPosition({}, {}, 1, True)

    position: PositionData = sink.positions[0]
    assert position.exchange == Exchange.SHFE
    assert position.direction == Direction.LONG
    assert position.yd_volume == 2
    assert position.volume == 2
    assert position.price == 3000
    assert position.frozen == 1
    assert position.pnl == 12.5


def test_ine_yd_position_matches_shfe(td_api: MiniTdApi, sink: Sink) -> None:
    add_contract(symbol="sc2510", exchange=Exchange.INE, size=10)
    td_api.onRspQryInvestorPosition(
        position_data(InstrumentID="sc2510", TodayPosition=0, YdPosition=1, Position=4, PositionCost=120000),
        {},
        1,
        False,
    )
    td_api.onRspQryInvestorPosition({}, {}, 1, True)

    assert sink.positions[0].yd_volume == 4
    assert sink.positions[0].volume == 4


def test_dce_yd_position_subtracts_today(td_api: MiniTdApi, sink: Sink) -> None:
    add_contract(symbol="m2501", exchange=Exchange.DCE, size=10)
    td_api.onRspQryInvestorPosition(
        position_data(InstrumentID="m2501", Position=5, TodayPosition=2, YdPosition=9),
        {},
        1,
        False,
    )
    td_api.onRspQryInvestorPosition({}, {}, 1, True)

    assert sink.positions[0].yd_volume == 3
    assert sink.positions[0].volume == 5


def test_shfe_today_position_does_not_set_yd(td_api: MiniTdApi, sink: Sink) -> None:
    add_contract()
    td_api.onRspQryInvestorPosition(position_data(TodayPosition=2, YdPosition=1), {}, 1, False)
    td_api.onRspQryInvestorPosition({}, {}, 1, True)

    assert sink.positions[0].yd_volume == 0


def test_empty_position_tail_flushes_cache(td_api: MiniTdApi, sink: Sink) -> None:
    add_contract()
    td_api.onRspQryInvestorPosition(position_data(), {}, 1, False)
    assert sink.positions == []

    td_api.onRspQryInvestorPosition({}, {}, 1, True)

    assert len(sink.positions) == 1
    assert td_api.positions == {}


def test_account_sums_frozen(td_api: MiniTdApi, sink: Sink) -> None:
    td_api.onRspQryTradingAccount({
        "AccountID": "u1",
        "Balance": 1000,
        "FrozenMargin": 10,
        "FrozenCash": 20,
        "FrozenCommission": 3,
        "Available": 800,
    }, {}, 1, True)

    account: AccountData = sink.accounts[0]
    assert account.accountid == "u1"
    assert account.balance == 1000
    assert account.frozen == 33
    assert account.available == 800


def test_account_without_id_is_ignored(td_api: MiniTdApi, sink: Sink) -> None:
    td_api.onRspQryTradingAccount({}, {}, 1, True)

    assert sink.accounts == []


def test_depth_without_time_is_ignored(md_api: MiniMdApi, sink: Sink) -> None:
    add_contract()
    md_api.onRtnDepthMarketData(depth_data(UpdateTime=""))

    assert sink.ticks == []


def test_depth_without_contract_is_ignored(md_api: MiniMdApi, sink: Sink) -> None:
    md_api.onRtnDepthMarketData(depth_data())

    assert sink.ticks == []


def test_dce_depth_uses_local_date(md_api: MiniMdApi, sink: Sink) -> None:
    add_contract(symbol="m2501", exchange=Exchange.DCE)
    md_api.current_date = "20250926"
    md_api.onRtnDepthMarketData(depth_data(InstrumentID="m2501", LastPrice=MAX_FLOAT))

    tick: TickData = sink.ticks[0]
    assert tick.datetime == datetime(2025, 9, 26, 9, 30, 0, 500000, tzinfo=CHINA_TZ)
    assert tick.last_price == 0
    assert tick.bid_price_2 == 0


def test_shfe_depth_uses_action_day_and_five_levels(md_api: MiniMdApi, sink: Sink) -> None:
    add_contract()
    md_api.current_date = "20250926"
    md_api.onRtnDepthMarketData(depth_data(BidVolume2=3, AskVolume2=4))

    tick: TickData = sink.ticks[0]
    assert tick.datetime == datetime(2020, 1, 1, 9, 30, 0, 500000, tzinfo=CHINA_TZ)
    assert tick.bid_price_5 == 2995
    assert tick.ask_volume_2 == 4


def test_subscribe_before_login_is_deferred(md_api: MiniMdApi, recorder: CallRecorder) -> None:
    md_api.subscribe(SubscribeRequest(symbol="rb2510", exchange=Exchange.SHFE))
    assert recorder.calls == []
    assert "rb2510" in md_api.subscribed

    md_api.onRspUserLogin({}, {"ErrorID": 0, "ErrorMsg": ""}, 1, True)

    assert recorder.calls == [("subscribeMarketData", "rb2510")]


def test_close_without_connection_does_not_exit(td_api: MiniTdApi, md_api: MiniMdApi, recorder: CallRecorder) -> None:
    td_api.close()
    md_api.close()

    assert recorder.calls == []
