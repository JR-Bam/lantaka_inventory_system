// For communicating with backend for adding equipment (added by zed)
document.getElementById('addProductModal').addEventListener('submit', function (event) {
    event.preventDefault(); // Prevent the default form submission

    const selectedUnit = document.getElementById('I_Unit').value;
    const unitId = unitMapping[selectedUnit];

    const productData = {
        I_Product: document.getElementById('I_Product').value,
        I_SN: document.getElementById('I_SN').value,
        I_Quantity: parseInt(document.getElementById('I_Quantity').value, 10),
        UNIT_ID: unitId,
        I_Location: document.getElementById('I_Location').value,
        E_Storage: document.getElementById('E_Storage').value
    };

    fetch('/api/equipment/add', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
            'Authorization': `Bearer ${localStorage.getItem("Lantaka-IMS-Sess-Token")}`
        },
        body: JSON.stringify(productData)
    })
        .then(response => response.json())
        .then(data => {
            if (data.status === "success") {
                alert("Product added successfully");
                closeAddProductModal();
                location.reload();
            } else {
                alert("Error adding product: " + data.message);
            }
        })
        .catch(error => {
            console.error("Error:", error);
            alert("An error occurred while adding the product.");
        });
});