document.getElementById('editModal').addEventListener('submit', function(event) {
    event.preventDefault();

    const changedEquipmentID = document.getElementById("editProductId").value;

    const changesMade = {
        "I_Product": (selectedEquipment["I_Product"] === document.getElementById("editProductName").value)? null : document.getElementById("editProductName").value,
        "I_SN": (selectedEquipment["I_SN"] === document.getElementById("editSerialNum").value)? null : document.getElementById("editSerialNum").value,
        "I_Quantity": (selectedEquipment["I_Quantity"] == document.getElementById("editQuantity").value)? null : parseInt(document.getElementById("editQuantity").value),
        "UNIT_ID": (selectedEquipment["UNIT_ID"] === document.getElementById("editQuantityUnit").value)? null : unitMapping[document.getElementById("editQuantityUnit").value],
        "I_Location": (selectedEquipment["I_Location"] === document.getElementById("editLocation").value)? null : document.getElementById("editLocation").value
    };

    const requestBody = { 
        "EquipmentID": parseInt(changedEquipmentID),
        "Updates": []
    };

    // Append each changed data to the Updates array
    Object.entries(changesMade).forEach(([key, value]) => {
        if (value != null) {
            requestBody["Updates"].push({[key]: value});
        }
    });

    console.log(JSON.stringify(requestBody));

    // Make a request to the server
    fetch('/api/equipment/edit', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify(requestBody)
    })
        .then(response => response.json())
        .then(data => {
            if (data.status == "success"){
                location.reload(); // TODO ! instead of doing this try to reset the equipment array and fetch the updated list.
            } else {
                error_div.innerHTML = alertCard("Something wrong happened when trying to edit the product.");
            }
            closeEditModal();
        })
        .catch(error => {
            closeEditModal();
            error_div.innerHTML = alertCard("Error: " + error);
        });
});


// Sample Format of SelectedEquipment
// Object { I_ID: 1, I_Product: "Pillow Case", I_SN: "N/A", I_Quantity: "274", UNIT_ID: "pieces", I_Location: "Sunken" }
// ​
// I_ID: 1
// ​
// I_Location: "Sunken"
// ​
// I_Product: "Pillow Case"
// ​
// I_Quantity: "274"
// ​
// I_SN: "N/A"
// ​
// UNIT_ID: "pieces"  
// * Note changedMade has this turned into the respective ID and will be the one to be passed instead of the name.
// ​
// <prototype>: Object { … }
// editProduct.js:6:13

