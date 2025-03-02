#!/bin/bash

# Configuration
SERVER_URL="http://localhost:9081"
CONCURRENT_CLIENTS=50
TEST_DURATION=10
LOG_DIR="tests/stress_logs"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
LOG_FILE="$LOG_DIR/stress_test_$TIMESTAMP.log"
UPLOAD_DIR="www/uploads"
TEST_DATA="This is test data for stress testing"

# Ensure clean state
cleanup() {
    echo "Cleaning up..."
    rm -f "$LOG_DIR"/client_*.log
    rm -rf "$UPLOAD_DIR"/*
}

# Create necessary directories
setup() {
    echo "Setting up test environment..."
    mkdir -p "$LOG_DIR"
    mkdir -p "$UPLOAD_DIR"
    chmod -R 777 "$UPLOAD_DIR"
    cleanup
}

# Function to send requests
send_requests() {
    local start_time=$(date +%s)
    local request_count=0
    local get_success=0
    local get_fail=0
    local post_success=0
    local post_fail=0
    local delete_success=0
    local delete_fail=0

    while true; do
        current_time=$(date +%s)
        if [ $((current_time - start_time)) -ge $TEST_DURATION ]; then
            break
        fi

        # Test GET request
        local get_response=$(curl -s -w "\n%{http_code}" "$SERVER_URL/" 2>/dev/null)
        local get_code=$(echo "$get_response" | tail -n1)
        if [ "$get_code" = "200" ]; then
            ((get_success++))
        else
            ((get_fail++))
        fi

        # Test POST request with data
        local test_file="test_${RANDOM}.txt"
        local post_response=$(curl -s -w "\n%{http_code}" -X POST \
            -H "Content-Type: text/plain" \
            -d "$TEST_DATA" \
            "$SERVER_URL/uploads/$test_file" -v 2>&1)
        local post_code=$(echo "$post_response" | tail -n1)
        
        echo "POST Response for $test_file:" >> "$LOG_FILE"
        echo "$post_response" >> "$LOG_FILE"
        
        if [ "$post_code" = "201" ] || [ "$post_code" = "200" ]; then
            ((post_success++))
            
            # Test DELETE request only for successful POSTs
            sleep 0.1  # Small delay to ensure file is written
            local delete_response=$(curl -s -w "\n%{http_code}" -X DELETE \
                "$SERVER_URL/uploads/$test_file" 2>/dev/null)
            local delete_code=$(echo "$delete_response" | tail -n1)
            
            if [ "$delete_code" = "200" ]; then
                ((delete_success++))
            else
                ((delete_fail++))
                echo "DELETE failed for $test_file with code $delete_code" >> "$LOG_FILE"
            fi
        else
            ((post_fail++))
            echo "POST failed for $test_file with code $post_code" >> "$LOG_FILE"
        fi

        ((request_count++))
    done

    echo "$get_success,$get_fail,$post_success,$post_fail,$delete_success,$delete_fail,$request_count"
}

# Main execution
echo "Starting stress test at $(date)" | tee -a "$LOG_FILE"
echo "Configuration:" | tee -a "$LOG_FILE"
echo "- Test duration: $TEST_DURATION seconds" | tee -a "$LOG_FILE"
echo "- Concurrent clients: $CONCURRENT_CLIENTS" | tee -a "$LOG_FILE"
echo "- Server URL: $SERVER_URL" | tee -a "$LOG_FILE"

# Initial setup
setup

# Check if server is already running
echo "Checking server status..."
if ! curl -s "$SERVER_URL/" > /dev/null 2>&1; then
    echo "Starting server..."
    ./webserv config/default.conf > /dev/null 2>&1 &
    SERVER_PID=$!
    echo "Waiting for server to start..."
    for i in {1..10}; do
        if curl -s "$SERVER_URL/" > /dev/null 2>&1; then
            echo "Server started successfully"
            break
        fi
        if [ $i -eq 10 ]; then
            echo "Server failed to start!"
            exit 1
        fi
        sleep 1
    done
fi

# Start concurrent clients
echo "Starting stress test with $CONCURRENT_CLIENTS concurrent clients..."
total_get_success=0
total_get_fail=0
total_post_success=0
total_post_fail=0
total_delete_success=0
total_delete_fail=0
total_requests=0

for ((i=1; i<=$CONCURRENT_CLIENTS; i++)); do
    send_requests > "$LOG_DIR/client_${i}.log" &
done

wait

# Process results
for ((i=1; i<=$CONCURRENT_CLIENTS; i++)); do
    if [ -f "$LOG_DIR/client_${i}.log" ]; then
        IFS=',' read -r gs gf ps pf ds df req < "$LOG_DIR/client_${i}.log"
        total_get_success=$((total_get_success + gs))
        total_get_fail=$((total_get_fail + gf))
        total_post_success=$((total_post_success + ps))
        total_post_fail=$((total_post_fail + pf))
        total_delete_success=$((total_delete_success + ds))
        total_delete_fail=$((total_delete_fail + df))
        total_requests=$((total_requests + req))
    fi
done

# Print results
echo -e "\nTest Results:"
echo "GET: $total_get_success successful, $total_get_fail failed"
echo "POST: $total_post_success successful, $total_post_fail failed"
echo "DELETE: $total_delete_success successful, $total_delete_fail failed"
echo "Total Requests: $total_requests"

# Final server check
if curl -s "$SERVER_URL/" > /dev/null 2>&1; then
    echo "Server is still responsive after stress test" | tee -a "$LOG_FILE"
    EXIT_CODE=0
else
    echo "Server is not responding after stress test!" | tee -a "$LOG_FILE"
    EXIT_CODE=1
fi

# Cleanup
cleanup

# Kill server if we started it
if [ -n "$SERVER_PID" ]; then
    kill $SERVER_PID 2>/dev/null
fi

exit $EXIT_CODE 
