function openModal(productId) {
    document.getElementById('deleteModal').style.display = 'block';
    document.getElementById('deleteProductId').value = productId;
}

function closeModal() {
    document.getElementById('deleteModal').style.display = 'none';
}

function resetSelectedEquipment(ID, NAME, QUANTITY, LOCATION, UNIT, SERIALNUM){
    selectedEquipment["I_ID"] = ID;
    selectedEquipment["I_Product"] = NAME;
    selectedEquipment["I_SN"] = SERIALNUM;
    selectedEquipment["I_Quantity"] = QUANTITY;
    selectedEquipment["UNIT_ID"] = UNIT.toLowerCase();
    selectedEquipment["I_Location"] = LOCATION;
}

function openEditModal(productId, productName, productQuantity, productLocation, productQuantityUnit, serialNum) {
    resetSelectedEquipment(productId, productName, productQuantity, productLocation, productQuantityUnit, serialNum);

    document.getElementById('editModal').style.display = 'block';
    document.getElementById('editProductId').value = productId;
    document.getElementById('editProductName').value = productName;
    document.getElementById('editQuantity').value = productQuantity;
    document.getElementById('editLocation').value = productLocation;
    document.getElementById('editSerialNum').value = serialNum;
    document.getElementById('editQuantityUnit').value = productQuantityUnit;
    
    // Set the unit dropdown
    const unitDropdown = document.getElementById('editUnit');
    for (let option of unitDropdown.options) {
        if (option.value === productQuantityUnit) {
            option.selected = true;
            break;
        }
    }
}

function closeEditModal() {
    document.getElementById('editModal').style.display = 'none';
}

// open the modal
function openAddProductModal() {
    document.getElementById('addProductModal').style.display = 'block';
}


// close the modal
function closeAddProductModal() {
    document.getElementById('addProductModal').style.display = 'none';
}

window.onclick = function(event) {
    const modal = document.getElementById('addProductModal');
    if (event.target === modal) {
        modal.style.display = 'none';
    }
}