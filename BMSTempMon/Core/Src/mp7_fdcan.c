/*
 * mp7_fdcan.c
 *
 *  Library for setting up FDCAN for mp7 projects
 *
 *  Last Modified: Jan 21, 2026
 *      By Author: gundlachj
 *
 *  Created on: Jan 3, 2026
 *      Author: gundlachj
 */

#include "mp7_fdcan.h"

/**
 * @brief   Configures the global FDCAN filter to reject all messages
 *          with IDs that do not match any applied filters.
 * @param   hfdcan address of FDCAN_HandleTypeDef used for
 *          configuring the FDCAN peripheral.
 * @retval  HAL_StatusTypeDef based on the completion
 *          of the filter configuration.
 */
HAL_StatusTypeDef MP7_FDCAN_ConfigureGlobalFilter(FDCAN_HandleTypeDef *hfdcan) {
  return HAL_FDCAN_ConfigGlobalFilter(hfdcan, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
}

/**
 *  @breif  Configures a new standard filter to send/reject messages based on an ID range.
 *          Note: Must be called after the FDCAN peripheral has been started.
 *  @param  hfdcan address of FDCAN_HandleTypeDef used for configuring the FDCAN peripheral.
 *  @param  FilterIDRangeLow lowest ID value that will be filtered.
 *  @param  FilterIDRangeHigh highest ID value that will be filtered.
 *  @param  FilterIndex index for the new filter for the list of standard filters.
 *          Note: FilterIndex must be less than StdFiltersNbr from the FDCAN Handler.
 *  @param  FilterDestinationConfig value of FDCAN_FILTER_xxx for what to do with the message.
 *  @retval HAL_StatusTypeDef based on the success of configuring the filter.
 *          If the filter ranges or index are invalid, then hfdcan.ErrorCode is set to
 *          HAL_FDCAN_ERROR_PARAM and a HAL_ERROR is returned.
 */
HAL_StatusTypeDef MP7_FDCAN_ConfigureFilter(FDCAN_HandleTypeDef *hfdcan, uint32_t FilterIDRangeLow,
                                            uint32_t FilterIDRangeHigh, uint32_t FilterIndex,
                                            uint32_t FilterDestinationConfig) {

  if (FilterIDRangeLow > FilterIDRangeHigh) {
    hfdcan->ErrorCode = HAL_FDCAN_ERROR_PARAM;
    return HAL_ERROR;
  }
  if (FilterIndex > hfdcan->Init.StdFiltersNbr) {
    hfdcan->ErrorCode = HAL_FDCAN_ERROR_PARAM;
    return HAL_ERROR;
  }

  FDCAN_FilterTypeDef filterConfig;
  filterConfig.IdType = FDCAN_STANDARD_ID;
  filterConfig.FilterConfig = FilterDestinationConfig;
  filterConfig.FilterType = FDCAN_FILTER_RANGE;
  filterConfig.FilterIndex = FilterIndex;
  filterConfig.FilterID1 = FilterIDRangeLow;
  filterConfig.FilterID2 = FilterIDRangeHigh;

  return HAL_FDCAN_ConfigFilter(hfdcan, &filterConfig);
}

/**
 * @brief   Configures a given FDCAN_TxHeaderTypeDef with the default MP7 configuration.
 * @param   TxHeader address of an FDCAN_TxHeaderTypeDef for configuring.
 * @param   Identifier specifies the ID to attach to the message.
 * @param   DataLength specifies the size of the message data.
 * @retval  1 if dataLength is greater than the max size for messages; otherwise, 0.
 */
uint8_t MP7_FDCAN_ConfigureTxHeader(FDCAN_TxHeaderTypeDef *TxHeader, uint32_t Identifier,
                                    uint32_t DataLength) {

  if (DataLength > FDCAN_DLC_BYTES_8) {
    return 1;
  }

  TxHeader->Identifier = Identifier;
  TxHeader->DataLength = DataLength;
  TxHeader->IdType = FDCAN_STANDARD_ID;
  TxHeader->FDFormat = FDCAN_CLASSIC_CAN;
  TxHeader->BitRateSwitch = FDCAN_BRS_OFF;
  TxHeader->TxFrameType = FDCAN_DATA_FRAME;
  TxHeader->ErrorStateIndicator = FDCAN_ESI_ACTIVE;

  return 0;
}
