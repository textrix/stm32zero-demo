# STM32ZERO-DEMO Design Document

## FDCAN Module

### Current Implementation (v1.1)

- Frame format configuration (Classic, FD_NO_BRS, FD_BRS)
- Flexible bitrate configuration with sample point control
- Filter types: single ID, dual ID, range, mask, accept-all
- Standard (11-bit) and Extended (29-bit) ID support via `send()` / `send_ext()`
- Bus state monitoring (ACTIVE, WARNING, PASSIVE, BUS_OFF)
- Error counter readout
- FIFO level monitoring

### Future Work

#### Multi-filter Support

현재 단일 필터만 지원. 향후 다중 필터 지원 추가 예정:

```cpp
// 다중 필터 API (미구현)
void add_filter_id(uint32_t id);
void add_filter_dual(uint32_t id1, uint32_t id2);
void add_filter_range(uint32_t min, uint32_t max);
void clear_filters();
```

**구현 시 고려사항:**
- STM32H7 FDCAN은 최대 128개 Standard 필터, 64개 Extended 필터 지원
- Message RAM 할당 관리 필요
- 필터 인덱스 자동 할당/관리

#### Remote Frame Support

```cpp
// Remote Frame API (미구현)
Status send_remote(uint16_t id, uint8_t dlc, uint32_t timeout_ms);
Status send_remote_ext(uint32_t id, uint8_t dlc, uint32_t timeout_ms);
```

**참고:** CAN-FD에서는 Remote Frame이 deprecated됨. Classic CAN 호환성을 위해서만 사용.

#### Loopback/Silent Mode

```cpp
// 진단 모드 (미구현)
void set_mode(Mode mode);  // NORMAL, LOOPBACK, SILENT, LOOPBACK_SILENT
```

#### Bus-Off Recovery

```cpp
// 자동 복구 (미구현)
void set_auto_recovery(bool enable);
void request_recovery();  // 수동 복구 요청
```

---

## Memory Layout

### Message RAM Allocation (기본값)

| 영역 | 크기 | 설명 |
|------|------|------|
| Standard Filters | 1 | 단일 Standard ID 필터 |
| Extended Filters | 1 | 단일 Extended ID 필터 |
| RX FIFO0 | 2 elements | 수신 버퍼 |
| TX FIFO | 2 elements | 송신 버퍼 |

---

## Timing Calculation

80MHz 클럭 기준 타이밍 계산:

```
Bitrate = Clock / (Prescaler * (1 + Seg1 + Seg2))
Sample Point = (1 + Seg1) / (1 + Seg1 + Seg2) * 100%
```

### 프리셋 타이밍 테이블 (87.5% Sample Point)

| Bitrate | Prescaler | Seg1 | Seg2 | SJW |
|---------|-----------|------|------|-----|
| 500K | 8 | 13 | 2 | 2 |
| 250K | 16 | 13 | 2 | 2 |
| 125K | 32 | 13 | 2 | 2 |

### Data Phase 타이밍 (75% Sample Point, CAN-FD)

| Bitrate | Prescaler | Seg1 | Seg2 | SJW |
|---------|-----------|------|------|-----|
| 2M | 2 | 14 | 5 | 5 |
| 4M | 1 | 14 | 5 | 5 |
| 5M | 2 | 5 | 2 | 1 |
