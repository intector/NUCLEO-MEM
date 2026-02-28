/**
 * Lab-Web-Demo - Numeric Value Formatting Helper
 * Copyright (c) 2025-2030 Intector
 *
 * Helper functions to format numeric values for display
 */

/**
 * Format a numeric value to specified decimal places
 * @param {number} value - The value to format
 * @param {number} decimals - Number of decimal places (default: 2)
 * @returns {string} - Formatted value string
 */
function formatNumericValue(value, decimals = 2) {
    if (value === null || value === undefined || isNaN(value)) {
        return '--';
    }
    return Number(value).toFixed(decimals);
}

/**
 * Update an element with a formatted numeric value
 * @param {string} elementId - The ID of the element to update
 * @param {number} value - The value to format and display
 * @param {number} decimals - Number of decimal places (default: 2)
 */
function updateNumericDisplay(elementId, value, decimals = 2) {
    const element = document.getElementById(elementId);
    if (element) {
        element.textContent = formatNumericValue(value, decimals);
    }
}
