// Implement search functionality
document.getElementById('search-button').onclick = function() {
    performSearch();
};

document.getElementById('search-input').addEventListener("keypress", function(event) {
    if (event.key === "Enter") {
        event.preventDefault(); // Prevent form submission
        performSearch(); // Trigger search function
    }
});

function performSearch() {
    var input = document.getElementById('search-input').value.toLowerCase();
    if (input) {
        currentEquipments = getEquipmentArray().filter(item => 
            Object.values(item).some(value => value.toString().toLowerCase().includes(input))
        );
    } else {
        currentEquipments = Equipments;
    }
    updateEquipmentVars();
    changePage(1);
}

const limit = 10;
var maxPages = 1;
var currentPage = 1;
var Equipments = [];

var currentEquipments = Equipments;

const paginate_nums_div = document.getElementById('paginate_nums');
const next_btn = document.getElementById('next');
const prev_btn = document.getElementById('prev');

const table = document.getElementById("rows");


function appendEquipment(equipment){
    Equipments.push(equipment);
}

function updateEquipmentVars(){
    maxPages = Math.ceil(getEquipmentArray().length / limit)
    paginate_nums_div.innerHTML = "";
    for (let num = 1; num <= maxPages; num++) {
        paginate_nums_div.innerHTML += `<a onclick="changePage(${num})" class="btn btn-primary paginate-nums">${num}</a>`;
    }

    checkButtons();
}

function changePage(page){
    currentPage = page;
    checkButtons();
    paginate(getEquipmentArray());
}

function getEquipmentArray(){
    return currentEquipments;
}

function paginate(array){
    const startIndex = (currentPage - 1) * limit;
    table.innerHTML = "";
    for (let equipment of array.slice(startIndex, startIndex + limit)){
        table.innerHTML += appendRow(
            equipment.ID, 
            equipment.Product_Name,
            (equipment.Serial_Number == 0)? "N/A" : equipment.Serial_Number,
            equipment.Quantity,
            equipment.Unit.name ?? "",
            equipment.Location
        )
    }
}

function onPrev(){
    if (currentPage > 1) changePage(currentPage - 1);
}

function onNext(){
    if (currentPage < maxPages) changePage(currentPage + 1);
}

function checkButtons(){
    if (currentPage == 1) {
        prev_btn.classList.remove("btn-primary");
        prev_btn.classList.add("d-none", "disabled");
    } else {
        prev_btn.classList.add("btn-primary");
        prev_btn.classList.remove("d-none", "disabled");
    }

    if (currentPage == maxPages) {
        next_btn.classList.remove("btn-primary");
        next_btn.classList.add("d-none", "disabled");
    } else {
        next_btn.classList.add("btn-primary");
        next_btn.classList.remove("d-none", "disabled");
    }
}

function appendRow(id, productName, serialNum, quantity, unit, location){
    if (!serialNum){
        serialNum = "N/A";
    }

    return `
    <tr>
        <td> ${productName} </td>
        <td> ${serialNum} </td>
        <td> ${quantity} ${unit} </td>
        <td> ${location} </td>
        <td>
            <button type="button" class="btn btn-primary" onclick="openEditModal(${id}, '${productName}', '${quantity}', '${location}', '${unit}')"><i class="fa-regular fa-pen-to-square"></i> Edit</button>
            <button type="button" class="btn btn-danger" onclick="openModal(${id})"><i class="fa-solid fa-trash"></i></button>
        </td>
    </tr>
    `;
}