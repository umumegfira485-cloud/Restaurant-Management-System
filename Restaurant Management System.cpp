-- Create Tables
CREATE TABLE Customers (
    customer_id NUMBER PRIMARY KEY,
    name VARCHAR2(100) NOT NULL,
    phone VARCHAR2(20),
    email VARCHAR2(100),
    join_date DATE DEFAULT SYSDATE
);

CREATE TABLE MenuItems (
    item_id NUMBER PRIMARY KEY,
    name VARCHAR2(100) NOT NULL,
    description VARCHAR2(500),
    price NUMBER(10,2) NOT NULL,
    category VARCHAR2(50),
    is_available NUMBER(1) DEFAULT 1
);

CREATE TABLE Orders (
    order_id NUMBER PRIMARY KEY,
    customer_id NUMBER,
    order_date TIMESTAMP DEFAULT SYSTIMESTAMP,
    status VARCHAR2(20) DEFAULT 'PENDING',
    total_amount NUMBER(10,2),
    notes VARCHAR2(500),
    CONSTRAINT fk_customer FOREIGN KEY (customer_id) REFERENCES Customers(customer_id)
);

CREATE TABLE OrderItems (
    order_item_id NUMBER PRIMARY KEY,
    order_id NUMBER,
    item_id NUMBER,
    quantity NUMBER NOT NULL,
    special_instructions VARCHAR2(500),
    item_price NUMBER(10,2),
    CONSTRAINT fk_order FOREIGN KEY (order_id) REFERENCES Orders(order_id),
    CONSTRAINT fk_item FOREIGN KEY (item_id) REFERENCES MenuItems(item_id)
);

CREATE TABLE Payments (
    payment_id NUMBER PRIMARY KEY,
    order_id NUMBER,
    amount NUMBER(10,2) NOT NULL,
    payment_date TIMESTAMP DEFAULT SYSTIMESTAMP,
    payment_method VARCHAR2(50),
    status VARCHAR2(20),
    CONSTRAINT fk_order_payment FOREIGN KEY (order_id) REFERENCES Orders(order_id)
);

-- Create Sequences
CREATE SEQUENCE customer_seq START WITH 1 INCREMENT BY 1;
CREATE SEQUENCE menu_item_seq START WITH 1 INCREMENT BY 1;
CREATE SEQUENCE order_seq START WITH 1 INCREMENT BY 1;
CREATE SEQUENCE order_item_seq START WITH 1 INCREMENT BY 1;
CREATE SEQUENCE payment_seq START WITH 1 INCREMENT BY 1;

-- 1. Customer CRUD Procedures
-- Create Customer
CREATE OR REPLACE PROCEDURE create_customer(
    p_name IN VARCHAR2,
    p_phone IN VARCHAR2,
    p_email IN VARCHAR2,
    p_customer_id OUT NUMBER
) AS
BEGIN
    SELECT customer_seq.NEXTVAL INTO p_customer_id FROM dual;
    INSERT INTO Customers(customer_id, name, phone, email)
    VALUES(p_customer_id, p_name, p_phone, p_email);
    COMMIT;
EXCEPTION
    WHEN OTHERS THEN
        ROLLBACK;
        RAISE;
END;
/

-- Read Customer
CREATE OR REPLACE PROCEDURE read_customer(
    p_customer_id IN NUMBER,
    p_name OUT VARCHAR2,
    p_phone OUT VARCHAR2,
    p_email OUT VARCHAR2,
    p_join_date OUT DATE
) AS
BEGIN
    SELECT name, phone, email, join_date
    INTO p_name, p_phone, p_email, p_join_date
    FROM Customers
    WHERE customer_id = p_customer_id;
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        RAISE_APPLICATION_ERROR(-20001, 'Customer not found');
END;
/

-- Update Customer
CREATE OR REPLACE PROCEDURE update_customer(
    p_customer_id IN NUMBER,
    p_name IN VARCHAR2,
    p_phone IN VARCHAR2,
    p_email IN VARCHAR2
) AS
BEGIN
    UPDATE Customers
    SET name = p_name,
        phone = p_phone,
        email = p_email
    WHERE customer_id = p_customer_id;
    IF SQL%ROWCOUNT = 0 THEN
        RAISE_APPLICATION_ERROR(-20002, 'No customer updated - ID not found');
    END IF;
    COMMIT;
EXCEPTION
    WHEN OTHERS THEN
        ROLLBACK;
        RAISE;
END;
/

-- Delete Customer (Soft Delete by Flagging)
CREATE OR REPLACE PROCEDURE delete_customer(
    p_customer_id IN NUMBER
) AS
    v_order_count NUMBER;
BEGIN
    SELECT COUNT(*) INTO v_order_count
    FROM Orders
    WHERE customer_id = p_customer_id;
    IF v_order_count > 0 THEN
        RAISE_APPLICATION_ERROR(-20003, 'Cannot delete customer with existing orders');
    END IF;
    DELETE FROM Customers WHERE customer_id = p_customer_id;
    IF SQL%ROWCOUNT = 0 THEN
        RAISE_APPLICATION_ERROR(-20004, 'Customer not found');
    END IF;
    COMMIT;
EXCEPTION
    WHEN OTHERS THEN
        ROLLBACK;
        RAISE;
END;
/

-- 2. Menu Item CRUD Procedures (REMOVED DUPLICATE SECTION)

-- 3. Order CRUD Procedures
-- Create Order
CREATE OR REPLACE PROCEDURE create_order(
    p_customer_id IN NUMBER,
    p_notes IN VARCHAR2 DEFAULT NULL,
    p_order_id OUT NUMBER
) AS
BEGIN
    SELECT order_seq.NEXTVAL INTO p_order_id FROM dual;
    INSERT INTO Orders(order_id, customer_id, notes)
    VALUES(p_order_id, p_customer_id, p_notes);
    COMMIT;
EXCEPTION
    WHEN OTHERS THEN
        ROLLBACK;
        RAISE;
END;
/

-- Read Order
CREATE OR REPLACE PROCEDURE read_order(
    p_order_id IN NUMBER,
    p_customer_id OUT NUMBER,
    p_order_date OUT TIMESTAMP,
    p_status OUT VARCHAR2,
    p_total_amount OUT NUMBER,
    p_notes OUT VARCHAR2
) AS
BEGIN
    SELECT customer_id, order_date, status, total_amount, notes
    INTO p_customer_id, p_order_date, p_status, p_total_amount, p_notes
    FROM Orders
    WHERE order_id = p_order_id;
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        RAISE_APPLICATION_ERROR(-20008, 'Order not found');
END;
/

-- Update Order
CREATE OR REPLACE PROCEDURE update_order(
    p_order_id IN NUMBER,
    p_customer_id IN NUMBER,
    p_status IN VARCHAR2,
    p_notes IN VARCHAR2
) AS
BEGIN
    UPDATE Orders
    SET customer_id = p_customer_id,
        status = p_status,
        notes = p_notes
    WHERE order_id = p_order_id;
    IF SQL%ROWCOUNT = 0 THEN
        RAISE_APPLICATION_ERROR(-20009, 'No order updated - ID not found');
    END IF;
    COMMIT;
EXCEPTION
    WHEN OTHERS THEN
        ROLLBACK;
        RAISE;
END;
/

-- Delete Order (with Items)
CREATE OR REPLACE PROCEDURE delete_order(
    p_order_id IN NUMBER
) AS
BEGIN
    DELETE FROM Payments WHERE order_id = p_order_id;
    DELETE FROM OrderItems WHERE order_id = p_order_id;
    DELETE FROM Orders WHERE order_id = p_order_id;
    IF SQL%ROWCOUNT = 0 THEN
        RAISE_APPLICATION_ERROR(-20010, 'Order not found');
    END IF;
    COMMIT;
EXCEPTION
    WHEN OTHERS THEN
        ROLLBACK;
        RAISE;
END;
/

-- Add Order Item (MISSING PROCEDURE ADDED)
CREATE OR REPLACE PROCEDURE add_order_item(
    p_order_id IN NUMBER,
    p_item_id IN NUMBER,
    p_quantity IN NUMBER,
    p_special_instructions IN VARCHAR2 DEFAULT NULL
) AS
    v_item_price NUMBER;
    v_order_item_id NUMBER;
BEGIN
    SELECT price INTO v_item_price FROM MenuItems WHERE item_id = p_item_id;
    SELECT order_item_seq.NEXTVAL INTO v_order_item_id FROM dual;
    INSERT INTO OrderItems(order_item_id, order_id, item_id, quantity, special_instructions, item_price)
    VALUES(v_order_item_id, p_order_id, p_item_id, p_quantity, p_special_instructions, v_item_price);
    COMMIT;
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        RAISE_APPLICATION_ERROR(-20013, 'Menu item not found');
    WHEN OTHERS THEN
        ROLLBACK;
        RAISE;
END;
/

-- Views
-- Active Orders View
CREATE OR REPLACE VIEW v_active_orders AS
SELECT o.order_id, c.name AS customer_name, o.order_date, o.status, o.total_amount
FROM Orders o
JOIN Customers c ON o.customer_id = c.customer_id
WHERE o.status IN ('PENDING', 'PREPARING', 'READY')
ORDER BY o.order_date;

-- Order Details View
CREATE OR REPLACE VIEW v_order_details AS
SELECT
    o.order_id,
    c.name AS customer_name,
    o.order_date,
    o.status,
    o.total_amount,
    mi.name AS item_name,
    oi.quantity,
    oi.item_price,
    (oi.quantity * oi.item_price) AS item_total,
    oi.special_instructions
FROM Orders o
JOIN Customers c ON o.customer_id = c.customer_id
JOIN OrderItems oi ON o.order_id = oi.order_id
JOIN MenuItems mi ON oi.item_id = mi.item_id;

-- Menu Availability View
CREATE OR REPLACE VIEW v_available_menu AS
SELECT item_id, name, description, price, category
FROM MenuItems
WHERE is_available = 1
ORDER BY category, name;

-- Triggers
-- Fixed: Compound Trigger for Order Total (Avoid Mutating Table)
CREATE OR REPLACE TRIGGER trg_update_order_total
FOR INSERT OR UPDATE OR DELETE ON OrderItems
COMPOUND TRIGGER
    TYPE t_order_tab IS TABLE OF NUMBER INDEX BY PLS_INTEGER;
    g_orders t_order_tab;

    AFTER EACH ROW IS
    BEGIN
        g_orders(:NEW.order_id) := :NEW.order_id;
        IF DELETING THEN
            g_orders(:OLD.order_id) := :OLD.order_id;
        END IF;
    END AFTER EACH ROW;

    AFTER STATEMENT IS
    BEGIN
        FOR idx IN 1..g_orders.COUNT LOOP
            UPDATE Orders
            SET total_amount = (
                SELECT SUM(quantity * item_price)
                FROM OrderItems
                WHERE order_id = g_orders(idx)
            )
            WHERE order_id = g_orders(idx);
        END LOOP;
    END AFTER STATEMENT;
END trg_update_order_total;
/

-- Item Availability Trigger
CREATE OR REPLACE TRIGGER trg_validate_item_availability
BEFORE INSERT OR UPDATE ON OrderItems
FOR EACH ROW
DECLARE
    v_available NUMBER;
BEGIN
    SELECT is_available INTO v_available
    FROM MenuItems
    WHERE item_id = :NEW.item_id;
    IF v_available = 0 THEN
        RAISE_APPLICATION_ERROR(-20011, 'Cannot add unavailable menu item to order');
    END IF;
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        RAISE_APPLICATION_ERROR(-20012, 'Invalid menu item ID');
END;
/

-- Order Status Log Table & Trigger
CREATE TABLE OrderStatusLog (
    log_id NUMBER PRIMARY KEY,
    order_id NUMBER,
    old_status VARCHAR2(20),
    new_status VARCHAR2(20),
    change_date TIMESTAMP DEFAULT SYSTIMESTAMP,
    changed_by VARCHAR2(100)
);
CREATE SEQUENCE status_log_seq START WITH 1 INCREMENT BY 1;

CREATE OR REPLACE TRIGGER trg_log_status_change
AFTER UPDATE OF status ON Orders
FOR EACH ROW
BEGIN
    IF :OLD.status != :NEW.status THEN
        INSERT INTO OrderStatusLog(log_id, order_id, old_status, new_status, changed_by)
        VALUES(status_log_seq.NEXTVAL, :NEW.order_id, :OLD.status, :NEW.status, USER);
    END IF;
END;
/

-- Functions
-- Calculate Order Total
CREATE OR REPLACE FUNCTION fn_calculate_order_total(
    p_order_id IN NUMBER
) RETURN NUMBER IS
    v_total NUMBER(10,2);
BEGIN
    SELECT SUM(quantity * item_price) INTO v_total
    FROM OrderItems
    WHERE order_id = p_order_id;
    RETURN NVL(v_total, 0);
EXCEPTION
    WHEN OTHERS THEN
        RETURN 0;
END;
/

-- Check Active Orders (Fixed: Return NUMBER instead of BOOLEAN)
CREATE OR REPLACE FUNCTION fn_customer_has_active_orders(
    p_customer_id IN NUMBER
) RETURN NUMBER IS
    v_count NUMBER;
BEGIN
    SELECT COUNT(*) INTO v_count
    FROM Orders
    WHERE customer_id = p_customer_id
    AND status NOT IN ('COMPLETED', 'CANCELLED');
    RETURN v_count;
EXCEPTION
    WHEN OTHERS THEN
        RETURN 0;
END;
/

-- Get Popular Items
CREATE OR REPLACE FUNCTION fn_get_popular_items(
    p_limit IN NUMBER DEFAULT 5
) RETURN SYS_REFCURSOR IS
    v_cursor SYS_REFCURSOR;
BEGIN
    OPEN v_cursor FOR
    SELECT mi.item_id, mi.name, SUM(oi.quantity) AS total_ordered
    FROM OrderItems oi
    JOIN MenuItems mi ON oi.item_id = mi.item_id
    JOIN Orders o ON oi.order_id = o.order_id
    WHERE o.status = 'COMPLETED'
    GROUP BY mi.item_id, mi.name
    ORDER BY total_ordered DESC
    FETCH FIRST p_limit ROWS ONLY;
    RETURN v_cursor;
END;
/

//