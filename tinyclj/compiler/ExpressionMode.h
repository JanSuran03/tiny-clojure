#pragma once

/**
 * Describes the mode in which the current code is executed
 */
enum class ExpressionMode {
    /**
     * The result of the current form is discarded
     * (it is embedded in a do-block and not the the last form in the block).
     */
    STATEMENT,
    /**
     * The result of the current form is stored inside a variable.
     */
    EXPRESSION,
    /**
     * The current form appears in the last position in a function body or a loop body
     * which means that its result is returned from the function or the loop.
     */
    RETURN
};
