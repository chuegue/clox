#include <string.h>

#include "interpreter.h"

int *error_code;

Literal *evaluate(Interpreter *interpreter, Expression *expr, int *error_code);

Interpreter *init_interpreter()
{
    Interpreter *new = calloc(1, sizeof(Interpreter));
    new->env = init_environment();
    return new;
}

void visitExpressionStatement(Interpreter *interpreter, Statement *stmt)
{
    evaluate(interpreter, stmt->expression1, error_code);
}

void visitPrintStatement(Interpreter *interpreter, Statement *stmt)
{
    Literal *value = evaluate(interpreter, stmt->expression1, error_code);
    print_literal(value);
}

void visitVarStatement(Interpreter *interpreter, Statement *stmt)
{
    Literal *value = calloc(1, sizeof(Literal));
    value->token_type = NIL;
    value->data.null = 1;
    if (stmt->expression2 != NULL)
    {
        value = evaluate(interpreter, stmt->expression2, error_code);
    }
    define_environment(interpreter->env, stmt->expression1->as.literal.data.string, value);
    return;
}

Literal *visitVariableExpression(Interpreter *interpreter, Expression *var_expr)
{
    return get_environment(interpreter->env, var_expr->as.variable.name->lexeme, error_code);
}

Literal *visitLiteralExpr(Interpreter *interpreter, Expression *expr)
{
    return &expr->as.literal;
}

Literal *visitGroupingExpr(Interpreter *interpreter, Expression *expr)
{
    return evaluate(interpreter, expr, error_code);
}

int isTruthy(Literal *object)
{
    if (object->token_type == NIL && object->data.null == 1)
    {
        return 0;
    }
    if (object->token_type == TRUE || object->token_type == FALSE)
    {
        return object->data.bool_val;
    }
    return 1;
}

Literal *visitUnaryExpr(Interpreter *interpreter, Expression *expr)
{
    Literal *right = evaluate(interpreter, expr->as.binary.right, error_code);
    switch (expr->as.binary.operator->type)
    {
    case BANG:
        Literal *ret = calloc(1, sizeof(Literal));
        ret->data.bool_val = !isTruthy(right);
        ret->token_type = ret->data.bool_val == 1 ? TRUE : FALSE;
        return ret;
        break;
    case MINUS:
        if (right->token_type != NUMBER)
        {
            fprintf(stderr, "Operand must be a number.\n");
            *error_code = 70;
            break;
        }
        *right->data.number *= -1;
        return right;
        break;
    }
    // Unreachable
    return NULL;
}

int isEqual(Literal *a, Literal *b)
{
    if (a->token_type == NIL && b->token_type == NIL)
    {
        return 1;
    }
    if (a->token_type == STRING && b->token_type == STRING)
    {
        return strcmp(a->data.string, b->data.string) == 0;
    }
    if (a->token_type == NUMBER && b->token_type == NUMBER)
    {
        return *(a->data.number) == *(b->data.number);
    }
    if ((a->token_type == TRUE && b->token_type == TRUE) || (a->token_type == FALSE && b->token_type == FALSE))
    {
        return 1;
    }
    if ((a->token_type == FALSE && b->token_type == TRUE) || (a->token_type == TRUE && b->token_type == FALSE))
    {
        return 0;
    }
    fprintf(stderr, "Trying to check if %s == %s\n", token_type_to_str(a->token_type), token_type_to_str(b->token_type));
    return 0;
}

void checkNumberOperands(Literal *left, Literal *right)
{
    // fprintf(stderr, "checkNumberOperands(%s, %s)\n",
    //         token_type_to_str(left->token_type),
    //         token_type_to_str(right->token_type));
    // Check if either operand is NOT a number
    if (left->token_type != NUMBER ||
        right->token_type != NUMBER)
    {
        fprintf(stderr, "Operands must be numbers.\n");
        *error_code = 70;
    }
    // Else, both are numbers (valid case), do nothing
}

Literal *visitBinaryExpr(Interpreter *interpreter, Expression *expr)
{
    Literal *left = evaluate(interpreter, expr->as.binary.left, error_code);
    Literal *right = evaluate(interpreter, expr->as.binary.right, error_code);
    if (left == NULL || right == NULL)
    {
        return NULL;
    }
    Literal *ret = calloc(1, sizeof(Literal));
    switch (expr->as.binary.operator->type)
    {
    case GREATER:
        checkNumberOperands(left, right);
        if (*error_code == 70)
        {
            return NULL;
        }
        ret->data.bool_val = *(left->data.number) > *(right->data.number);
        ret->token_type = ret->data.bool_val == 1 ? TRUE : FALSE;
        return ret;
    case GREATER_EQUAL:
        checkNumberOperands(left, right);
        if (*error_code == 70)
        {
            return NULL;
        }
        ret->data.bool_val = *(left->data.number) >= *(right->data.number);
        ret->token_type = ret->data.bool_val == 1 ? TRUE : FALSE;
        return ret;
    case LESS:
        checkNumberOperands(left, right);
        if (*error_code == 70)
        {
            return NULL;
        }
        ret->data.bool_val = *(left->data.number) < *(right->data.number);
        ret->token_type = ret->data.bool_val == 1 ? TRUE : FALSE;
        return ret;
    case LESS_EQUAL:
        checkNumberOperands(left, right);
        if (*error_code == 70)
        {
            return NULL;
        }
        ret->data.bool_val = *(left->data.number) <= *(right->data.number);
        ret->token_type = ret->data.bool_val == 1 ? TRUE : FALSE;
        return ret;
    case EQUAL_EQUAL:
        ret->data.bool_val = isEqual(left, right);
        ret->token_type = ret->data.bool_val == 1 ? TRUE : FALSE;
        return ret;
        break;
    case BANG_EQUAL:
        ret->data.bool_val = !isEqual(left, right);
        ret->token_type = ret->data.bool_val == 1 ? TRUE : FALSE;
        return ret;
        break;
    case MINUS:
        checkNumberOperands(left, right);
        if (*error_code == 70)
        {
            return NULL;
        }
        ret->token_type = NUMBER;
        ret->data.number = calloc(1, sizeof(double));
        *(ret->data.number) = *(left->data.number) - *(right->data.number);
        return ret;
        break;
    case PLUS:
        if (left->token_type == NUMBER && right->token_type == NUMBER)
        {
            ret->token_type = NUMBER;
            ret->data.number = calloc(1, sizeof(double));
            *(ret->data.number) = *(left->data.number) + *(right->data.number);
            return ret;
        }
        if (left->token_type == STRING && right->token_type == STRING)
        {
            ret->token_type = STRING;
            ret->data.string = calloc(strlen(left->data.string) + strlen(right->data.string) + 1, sizeof(char));
            strcpy(ret->data.string, left->data.string);
            strcpy(ret->data.string + strlen(left->data.string), right->data.string);
            return ret;
        }
        checkNumberOperands(left, right);
        if (*error_code == 70)
        {
            return NULL;
        }
        break;
    case SLASH:
        checkNumberOperands(left, right);
        if (*error_code == 70)
        {
            return NULL;
        }
        ret->token_type = NUMBER;
        ret->data.number = calloc(1, sizeof(double));
        *(ret->data.number) = *(left->data.number) / *(right->data.number);
        return ret;
        break;
    case STAR:
        checkNumberOperands(left, right);
        if (*error_code == 70)
        {
            return NULL;
        }
        ret->token_type = NUMBER;
        ret->data.number = calloc(1, sizeof(double));
        *(ret->data.number) = *(left->data.number) * *(right->data.number);
        return ret;
        break;
    default:
        return NULL;
        break;
    }
    return NULL;
}

Literal *evaluate(Interpreter *interpreter, Expression *expr, int *error_code_param)
{
    if (*error_code_param != 0)
    {
        return NULL;
    }
    error_code = error_code_param;
    switch (expr->type)
    {
    case EXPR_LITERAL:
        return visitLiteralExpr(interpreter, expr);
        break;
    case EXPR_BINARY:
        return visitBinaryExpr(interpreter, expr);
        break;
    case EXPR_GROUPING:
        return visitGroupingExpr(interpreter, expr->as.binary.left);
        break;
    case EXPR_UNARY:
        return visitUnaryExpr(interpreter, expr);
        break;
    case EXPR_VARIABLE:
        return visitVariableExpression(interpreter, expr);
        break;

    default:
        break;
    }
    printf("HOW THE FUCK DID YOU GET HERE\n");
    return NULL;
}

void execute(Interpreter *interpreter, Statement *statement, int *error_code_param)
{
    if (*error_code_param != 0)
    {
        return;
    }
    error_code = error_code_param;
    switch (statement->type)
    {
    case STMT_PRINT:
        visitPrintStatement(interpreter, statement);
        break;
    case STMT_EXPR:
        visitExpressionStatement(interpreter, statement);
        break;
    case STMT_VAR:
        visitVarStatement(interpreter, statement);
        break;
    default:
    printf("HOW THE FUCK DID YOU GET HERE\n");
        break;
    }
}

void interpret(Statement **statements, size_t len_statements, int *error_code_param)
{
    Interpreter *interpreter = init_interpreter();
    for (size_t i = 0; i < len_statements; i++)
    {
        execute(interpreter, statements[i], error_code_param);
    }
}